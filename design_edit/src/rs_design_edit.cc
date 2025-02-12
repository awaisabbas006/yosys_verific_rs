/**
 * @file rs_design_edit.cc
 * @author Behzad Mehmood (behzadmehmood82@gmail.com)
 * @author Manadher Kharroubi (manadher@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-02
 *
 * @copyright Copyright (c) 2024
 */
#include "backends/rtlil/rtlil_backend.h"
#include "kernel/celltypes.h"
#include "kernel/ff.h"
#include "kernel/ffinit.h"
#include "kernel/log.h"
#include "kernel/mem.h"
#include "kernel/register.h"
#include "kernel/rtlil.h"
#include "kernel/yosys.h"
#include "primitives_extractor.h"
#include "rs_design_edit.h"
#include "rs_primitive.h"
#include "netlist_checker.h"
#include <json.hpp>
#include <chrono>

#ifdef PRODUCTION_BUILD
#include "License_manager.hpp"
#endif

int DSP_COUNTER;
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

#define XSTR(val) #val
#define STR(val) XSTR(val)

#ifndef PASS_NAME
#define PASS_NAME design_edit
#endif

#define GENESIS_DIR genesis
#define GENESIS_2_DIR genesis2
#define GENESIS_3_DIR genesis3
#define COMMON_DIR common
#define VERSION_MAJOR 0 // 0 - beta
#define VERSION_MINOR 0
#define VERSION_PATCH 1

using json = nlohmann::json;
using namespace std::chrono;

USING_YOSYS_NAMESPACE
using namespace RTLIL;

const std::vector<std::string> IN_PORTS = {"I", "I_P", "I_N", "D"};
const std::vector<std::string> DATA_OUT_PORTS = {"O", "O_P", "O_N", "Q"};
const std::vector<std::string> DATA_CLK_OUT_PORTS = {"O", "O_P", "O_N", "Q", "CLK_OUT", "CLK_OUT_DIV2", "CLK_OUT_DIV3", "CLK_OUT_DIV4", "FAST_CLK", "OUTPUT_CLK"};

struct DesignEditRapidSilicon : public ScriptPass {
  DesignEditRapidSilicon()
      : ScriptPass("design_edit", "Netlist Editing Tool") {}
  
  ~DesignEditRapidSilicon() {
    while (pins.size()) {
      delete pins.back();
      pins.pop_back();
    }
  }

  void help() override {
    log("\n");
    log("This command runs Netlist editing tool\n");
    log("\n");
    log("    -verilog <file>\n");
    log("        Write the design to the specified verilog file. writing of an "
        "output file\n");
    log("        is omitted if this parameter is not specified.\n");
    log("\n");
    log("\n");
  }

  std::vector<Cell *> remove_prims;
  std::vector<Cell *> remove_fab_prims;                  // TODO : change to unoredred set later
  std::vector<Cell *> remove_non_prims;
  std::vector<Cell *> remove_wrapper_cells;
  std::unordered_set<Wire *> wires_interface;
  std::unordered_set<Wire *> del_ins;
  std::unordered_set<Wire *> del_outs;
  std::unordered_set<Wire *> del_interface_wires;
  std::unordered_set<Wire *> del_wrapper_wires;
  std::unordered_set<Wire *> del_unused;
  std::set<std::pair<Yosys::RTLIL::SigSpec, Yosys::RTLIL::SigSpec>> connections_to_remove;
  std::unordered_set<Wire *> orig_intermediate_wires;
  std::unordered_set<Wire *> interface_intermediate_wires;
  std::map<Yosys::RTLIL::SigBit, Yosys::RTLIL::SigBit> wrapper_conns;
  std::map<RTLIL::SigBit, std::vector<RTLIL::Wire *>> io_prim_conn, intf_prim_conn;
  std::map<RTLIL::SigBit, RTLIL::SigBit> inout_conn_map;
  std::map<Yosys::RTLIL::SigBit, Yosys::RTLIL::SigBit> ifab_sig_map;
  std::map<RTLIL::SigBit, std::vector<RTLIL::SigBit>> ofab_sig_map, ofab_conns;
  pool<SigBit> prim_out_bits;
  pool<SigBit> unused_prim_outs;
  pool<SigBit> used_bits;
  pool<SigBit> orig_ins, orig_outs, fab_outs, fab_ins;

  RTLIL::Design *_design;
  RTLIL::Design *new_design = new RTLIL::Design;
  primitives_data io_prim;

  void clear_flags() override { wrapper_files = {}; }

  std::vector<std::string> tokenizeString(const std::string &input) {
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
      tokens.push_back(token);
    }

    return tokens;
  }
  
  pin_data* get_pin(std::string name, bool create_new_if_not_exist = true) {
    pin_data* pin = nullptr;
    for (auto& p : pins) {
      if (p->_name == name) {
        pin = p;
        break;
      }
    }
    if (pin == nullptr && create_new_if_not_exist) {
      pin = new pin_data(name);
      pins.push_back(pin);
    }
    return pin;
  }
  void processSdcFile(std::istream &input) {
    std::string line;
    while (std::getline(input, line)) {
      std::vector<std::string> tokens = tokenizeString(line);
      if (!tokens.size())
        continue;
      if ("set_property" == tokens[0]) {
        if (tokens.size() == 4) {
          pin_data* pin = get_pin(tokens[3]);
          log_assert(pin != nullptr);
          pin->_properties[tokens[1]] = tokens[2];
        }
      } else if ("set_pin_loc" == tokens[0]) {
        if (tokens.size() < 3 || tokens.size() > 4) continue;
        pin_data* pin = get_pin(tokens[1]);
        log_assert(pin != nullptr);
        pin->_location = tokens[2];
        if (tokens.size() == 4) {
          pin->_internal_pin = tokens[3];
        }
      }
    }
  }

  std::string id(RTLIL::IdString internal_id)
  {
    const char *str = internal_id.c_str();
    return std::string(str);
  }

  std::string process_connection(const RTLIL::SigChunk &chunk) {
    std::stringstream output;
    if (chunk.width == chunk.wire->width && chunk.offset == 0) {
      output << id(chunk.wire->name);
    } else if (chunk.width == 1) {
      if (chunk.wire->upto)
        output << id(chunk.wire->name) << "[" << (chunk.wire->width - chunk.offset - 1) + chunk.wire->start_offset << "]";
      else
        output << id(chunk.wire->name) << "[" << chunk.offset + chunk.wire->start_offset << "]";
    } else {
      if (chunk.wire->upto)
        output << id(chunk.wire->name) << "[" << (chunk.wire->width - (chunk.offset + chunk.width - 1) - 1) + chunk.wire->start_offset << ":" <<
                  (chunk.wire->width - chunk.offset - 1) + chunk.wire->start_offset << "]";
      else
        output << id(chunk.wire->name) << "[" << (chunk.offset + chunk.width - 1) + chunk.wire->start_offset << ":" <<
                  chunk.offset + chunk.wire->start_offset << "]";
    }
    return output.str();
  }

  void dump_io_config_json(Module* mod, std::string file) {
    std::ofstream json_file(file.c_str());
    json instances = json::object();
    json instances_array = json::array();
    for(auto cell : mod->cells()) {
      json instance_object;
      instance_object["module"] = remove_backslashes(cell->type.str());
      instance_object["name"] = remove_backslashes(cell->name.str());
      for(auto conn : cell->connections()) {
        std::string port_name = remove_backslashes(conn.first.str());
        std::vector<std::string> signals;
        PRIMITIVES_EXTRACTOR::get_signals(conn.second, signals);
        std::string connection = "";
        for (size_t i = 0; i < signals.size(); i++) {
          signals[i] = remove_backslashes(signals[i]);
        }
        if (signals.size() == 0) {
          instance_object["connectivity"][port_name] = "";
        } else if (signals.size() == 1) {
          instance_object["connectivity"][port_name] = signals[0];
          connection = signals[0];
        } else {
          // array of signals
          instance_object["connectivity"][port_name] = nlohmann::json::array();
          for (auto& s : signals) {
            instance_object["connectivity"][port_name].push_back(s);
          }
        }
      }
      instances_array.push_back(instance_object);
    }
    // enhancement to auto create wire primitives
    size_t i = 0;
    for (auto it : mod->connections()) {
      std::vector<std::string> lefts;
      std::vector<std::string> rights;
      PRIMITIVES_EXTRACTOR::get_signals(it.first, lefts);
      PRIMITIVES_EXTRACTOR::get_signals(it.second, rights);
      log_assert(lefts.size() == rights.size());
      // break the bus into bit by bit
      for (size_t j = 0; j < lefts.size(); j++) {
        json instance_object;
        instance_object["module"] = (std::string)("WIRE");
        instance_object["name"] = (std::string)(stringf("wire%ld", i));
        instance_object["connectivity"]["I"] = remove_backslashes(rights[j]);
        instance_object["connectivity"]["O"] = remove_backslashes(lefts[j]);
        instances_array.push_back(instance_object);
        i++;
      }
    }
#if 0
    // Starting by marking all the "port" primitives
    // IO bitstream generation will only need a unique name to know which primitives are linked together, any name will do
    // But it does not work for other flow, which they use linked_object name as port name
    i = 0;
    std::vector<std::string> port_primitives = {"I_BUF", "I_BUF_DS", "O_BUF", "O_BUFT", "O_BUF_DS", "O_BUFT_DS", "BOOT_CLOCK"};
    for (auto& inst : instances_array) {
      if (std::find(port_primitives.begin(), port_primitives.end(), inst["module"]) != port_primitives.end()) {
        inst["linked_object"] = std::string(stringf("object%ld", i));
        i++;
      }
    }
#else
    // Use the port name to link the instance
    for (const RTLIL::Wire* wire : mod->wires()) {
      // We can use one line code: !wire->port_input && !wire->port_output
      // But prefer list of all the valid possible of Input, Output, Inout
      std::string dir = "";
      if (wire->port_input && !wire->port_output) {
        dir = "IN";
      } else if (!wire->port_input && wire->port_output) {
        dir = "OUT";
      } else if (wire->port_input && wire->port_output) {
        dir = "INOUT";
      }
      if (dir.size()) {
        for (int index = 0; index < wire->width; index++) {
          std::string portname = wire->name.str();
          if (wire->width > 1) {
            portname = stringf("%s[%d]", wire->name.c_str(), wire->start_offset + index);
          }
          portname = remove_backslashes(portname);
          if (dir == "INOUT") {
            link_instance(true, instances_array, portname, portname, "IN", 0, true, DATA_OUT_PORTS);
            link_instance(false, instances_array, portname, portname, "OUT", 0, true, DATA_OUT_PORTS);
          } else {
            link_instance(dir == "IN", instances_array, portname, portname, dir, 0, true, DATA_OUT_PORTS);
          }
        }
      }
    }
#endif
    // Handle pure-data
    link_instance_recursively(instances_array, 0, DATA_OUT_PORTS);
    // Handle clock
    for (std::string module : std::vector<std::string>({"BOOT_CLOCK", "FCLK_BUF"})) {
      i = 0;
      for (auto& inst : instances_array) {
        if (inst["module"] == module) {
          if (inst["connectivity"].contains("O")) {
            inst["linked_object"] = inst["connectivity"]["O"];
          } else {
            inst["linked_object"] = stringf("%s#%ld", module.c_str(), i);
          }
          inst["direction"] = "IN";
          inst["index"] = 0;
          i++;
        }
      }
    }
    // Handle clock-data
    link_instance_recursively(instances_array, 1, DATA_CLK_OUT_PORTS);
    instances["instances"] = instances_array;
    if (json_file.is_open()) {
      json_file << std::setw(4) << instances << std::endl;
      json_file.close();
    }
  }

#if 1
  void link_instance_recursively(json& instances_array, int retry_start_index, const std::vector<std::string>& OUT_PORTS) {
    log_assert(retry_start_index == 0 || retry_start_index == 1);
    // Compare to original link_instance_recursively() which had been commented out:
    //    This code does not need to specially handle I_BUF_DS and O_BUF_DS, O_BUFT_DS because
    //    netlist editor had removed the extra wire
    while (true) {
      // Recursively marks other primitives
      size_t linked = 0;
      for (auto& inst : instances_array) {
        if (inst.contains("linked_object")) {
          for (auto& iter : inst["connectivity"].items()) {
            bool src_is_in = std::find(IN_PORTS.begin(), IN_PORTS.end(), (std::string)(iter.key())) != IN_PORTS.end();
            bool src_is_out = std::find(OUT_PORTS.begin(), OUT_PORTS.end(), (std::string)(iter.key())) !=  OUT_PORTS.end();
            if (src_is_in || src_is_out) {
              log_assert((src_is_in & src_is_out) == false);
              nlohmann::json signals = iter.value();
              if (signals.is_string()) {
                signals = nlohmann::json::array();
                signals.push_back((std::string)(iter.value()));
              } else {
                log_assert(signals.is_array());
              }
              for (auto& s : signals) {
                std::string net = (std::string)(s);
                if (!PRIMITIVES_EXTRACTOR::is_real_net(net)) {
                  continue;
                }
                // dont set allow_dual_name=true, it might become infinite loop
                linked += link_instance(!src_is_in, instances_array, inst["linked_object"], net, 
                                        inst["direction"], uint32_t(inst["index"]) + 1, false, OUT_PORTS);
              }
            }
          }
        }
      }
      if (linked == 0) {
        // 1st time: we do not need recursive loop. One time is enough
        // 2nd time: we need recursive until we cannot link anymore
        break;
      }
    }
  }
#else
  void link_instance_recursively(json& instances_array, int retry_start_index, const std::vector<std::string>& OUT_PORTS) {
    log_assert(retry_start_index == 0 || retry_start_index == 1);
    // Special case for I_BUF_DS and O_BUF_DS, O_BUFT_DS, because they have multiple objects
    // We need to loop this recursive loop twice
    for (int i = retry_start_index; i < 2; i++) {
      // first time : only link I_BUF_DS and O_BUF_DS, O_BUFT_DS (before they are used to link other instance)
      //              because the name needs to be "p+n"
      // second time: link the rest
      while (true) {
        // Recursively marks other primitives
        size_t linked = 0;
        for (auto& inst : instances_array) {
          if (inst.contains("linked_object")) {
            for (auto& iter : inst["connectivity"].items()) {
              bool src_is_in = std::find(IN_PORTS.begin(), IN_PORTS.end(), (std::string)(iter.key())) != IN_PORTS.end();
              bool src_is_out = std::find(OUT_PORTS.begin(), OUT_PORTS.end(), (std::string)(iter.key())) !=  OUT_PORTS.end();
              if (src_is_in || src_is_out) {
                log_assert((src_is_in & src_is_out) == false);
                nlohmann::json signals = iter.value();
                if (signals.is_string()) {
                  signals = nlohmann::json::array();
                  signals.push_back((std::string)(iter.value()));
                } else {
                  log_assert(signals.is_array());
                }
                for (auto& s : signals) {
                  std::string net = (std::string)(s);
                  if (!PRIMITIVES_EXTRACTOR::is_real_net(net)) {
                    continue;
                  }
                  if (i == 0) {
                    linked += link_instance(!src_is_in, instances_array, inst["linked_object"], net, 
                                            inst["direction"], uint32_t(inst["index"]) + 1, true, OUT_PORTS,
                                            {"I_BUF_DS", "O_BUF_DS", "O_BUFT_DS"});
                  } else {
                    // dont set allow_dual_name=true, it might become infinite loop
                    linked += link_instance(!src_is_in, instances_array, inst["linked_object"], net, 
                                            inst["direction"], uint32_t(inst["index"]) + 1, false, OUT_PORTS);
                  }
                }
              }
            }
          }
        }
        if (i == 0 || linked == 0) {
          // 1st time: we do not need recursive loop. One time is enough
          // 2nd time: we need recursive until we cannot link anymore
          break;
        }
      }
    }
  }
#endif
  
  size_t link_instance(bool use_in_port, json& instances_array, const std::string& object, 
                        const std::string& net, const std::string& direction, uint32_t index, bool allow_dual_name, 
                        const std::vector<std::string>& OUT_PORTS, std::vector<std::string> search_modules = {}) {
    size_t linked = 0;
    for (auto& inst : instances_array) {
      // Only if this instance had not been linked
      if (search_modules.size() > 0 &&
          std::find(search_modules.begin(), search_modules.end(), inst["module"]) == search_modules.end()) {
        continue;
      }
      if (!inst.contains("linked_object") || allow_dual_name) {
        for (auto& iter : inst["connectivity"].items()) {
          if (!iter.value().is_string()) {
            continue;
          }
          std::string inst_net = (std::string)(iter.value());
          if (!PRIMITIVES_EXTRACTOR::is_real_net(inst_net)) {
            continue;
          }
          bool match = false;
          if (use_in_port && 
              (std::find(IN_PORTS.begin(), IN_PORTS.end(), (std::string)(iter.key())) != IN_PORTS.end() || 
               (inst["module"] == "PLL" && (std::string)(iter.key()) == "CLK_IN"))) {
            match = true;
          }
          if (!use_in_port && 
              (std::find(OUT_PORTS.begin(), OUT_PORTS.end(), (std::string)(iter.key())) !=  OUT_PORTS.end())) {
            match = true;
          }
          if (match) {
            if (inst_net == net) {
              if (inst.contains("linked_object")) {
                inst["linked_object"] = stringf("%s+%s", ((std::string)(inst["linked_object"])).c_str(), object.c_str());
              } else {
                inst["linked_object"] = object;
              }
              inst["direction"] = direction;
              inst["index"] = index;
              linked++;
              break;
            }
          }
        }
      }
    }
    return linked;
  }

  std::string remove_backslashes(const std::string &input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
      if (c != '\\') {
        result.push_back(c); 
      }
    }
    return result;
  }

  void categorize_primitives()
  {
    for (const std::string& primitive : primitives)
    {
      if (primitive.substr(0, 2) == "O_")
      {
        out_prims.insert(primitive);
      } else if (primitive.substr(0, 9) == "SOC_FPGA_")
      {
        soc_intf_prims.insert(primitive);
      }
    }
  }

  void delete_cells(Module *module, vector<Cell *> cells) {
    for (auto cell : cells) {
      module->remove(cell);
    }
  }

  void delete_wires(Module *module, std::unordered_set<Wire *> wires) {
    for (auto wire : wires) {
      std::string wire_name = wire->name.str();
      if (keep_wires.find(wire_name) == keep_wires.end()) {
        module->remove({wire});
      }
    }
  }

  void remove_io_fab_prim(Module *mod)
  {
    for(auto cell : mod->cells())
    {
      if (cell->type == RTLIL::escape_id("I_FAB"))
      {
        SigBit in_bit, out_bit;
        for (auto &conn : cell->connections())
        {
          IdString portName = conn.first;
          if (cell->input(portName))
          {
            in_bit = conn.second;
          } else {
            out_bit = conn.second;
          }
        }
        if (in_bit.wire != nullptr)
        {
          remove_fab_prims.push_back(cell);
          if (fab_ins.count(in_bit) && fab_outs.count(out_bit))
          {
            RTLIL::SigSig new_conn;
            new_conn.first = out_bit;
            new_conn.second = in_bit;
            mod->connect(new_conn);
          } else {
            ifab_sig_map.insert(std::make_pair(out_bit, in_bit));
          }
        }
      }
    }

    for(auto cell : mod->cells())
    {
      if (cell->type == RTLIL::escape_id("O_FAB"))
      {
        SigBit in_bit, out_bit;
        for (auto &conn : cell->connections())
        {
          IdString portName = conn.first;
          if (cell->input(portName))
          {
            in_bit = conn.second;
          } else {
            out_bit = conn.second;
          }
        }
        if (in_bit.wire != nullptr)
        {
          remove_fab_prims.push_back(cell);
          if (fab_ins.count(in_bit) && fab_outs.count(out_bit))
          {
            RTLIL::SigSig new_conn;
            new_conn.first = out_bit;
            new_conn.second = in_bit;
            mod->connect(new_conn);
          } else if(ifab_sig_map.count(in_bit))
          {
            RTLIL::SigSig new_conn;
            new_conn.first = out_bit;
            new_conn.second = ifab_sig_map[in_bit];
            mod->connect(new_conn);
          } else {
            auto it = ofab_sig_map.find(in_bit);

            if (it != ofab_sig_map.end()) {
              it->second.push_back(out_bit);
            } else {
              std::vector<RTLIL::SigBit> out_bits;
              out_bits.push_back(out_bit);
              ofab_sig_map.insert({in_bit, out_bits});
            }
          }
        }
      }
    }

    delete_cells(mod, remove_fab_prims);

    for (auto cell : mod->cells())
    {
      if (cell->type == RTLIL::escape_id("O_FAB") ||
        cell->type == RTLIL::escape_id("I_FAB")) continue;

      for (auto conn : cell->connections())
      {
        IdString portName = conn.first;
        bool unset_port = true;
        RTLIL::SigSpec sigspec;
        for (SigBit bit : conn.second)
        {
          if (ofab_sig_map.count(bit))
          {
            const std::vector<RTLIL::SigBit> outbits = ofab_sig_map[bit];
            if(outbits.size() < 1) sigspec.append(bit);
            if(outbits.size() == 1)
            {
              if (unset_port)
              {
                cell->unsetPort(portName);
                unset_port = false;
              }
              sigspec.append(outbits[0]);
            } else if (outbits.size() > 1)
            {
              sigspec.append(bit);
              if (ofab_conns.find(bit) == ofab_conns.end())
              {
                ofab_conns.insert({bit, outbits});
              }
            }
          } else {
            sigspec.append(bit);
          }
        }
        if (!unset_port) cell->setPort(portName, sigspec);
      }

      for (auto conn : cell->connections())
      {
        IdString portName = conn.first;
        bool unset_port = true;
        RTLIL::SigSpec sigspec;
        for (SigBit bit : conn.second)
        {
          if (ifab_sig_map.count(bit))
          {
            if (unset_port)
            {
              cell->unsetPort(portName);
              unset_port = false;
            }
            sigspec.append(ifab_sig_map[bit]);
          } else {
            sigspec.append(bit);
          }
        }
        if (!unset_port) cell->setPort(portName, sigspec);
      }
    }

    for (const auto& ofab_conn : ofab_conns)
    {
      const std::vector<RTLIL::SigBit> out_bits = ofab_conn.second;
      if(out_bits.size() <= 1) continue;
      for(const auto& out_bit : out_bits)
      {
        RTLIL::SigSig new_conn;
        new_conn.first = out_bit;
        new_conn.second = ofab_conn.first;
        mod->connect(new_conn);
      }
    }
  }

  void rem_extra_wires(Module *module)
  {
    pool<SigBit> bits_used;
    std::unordered_set<Wire *> del_wires;

    for(auto cell : module->cells())
    {
      for (auto &conn : cell->connections())
      {
        for (SigBit bit : conn.second)
        {
          if (bit.wire != nullptr) bits_used.insert(bit);
        }
      }
    }

    for(auto &conn : module->connections())
    {
      for (SigBit bit : conn.second)
      {
        if (bit.wire != nullptr) bits_used.insert(bit);
      }
      for (SigBit bit : conn.first)
      {
        if (bit.wire != nullptr) bits_used.insert(bit);
      }
    }

    for (auto wire : module->wires())
    {
      RTLIL::SigSpec wire_ = wire;
      for (auto bit : wire_)
      {
        if(!bits_used.count(bit) && !bit.wire->port_output && !bit.wire->port_input)
        {
          if(bit.wire->width == 1) del_wires.insert(bit.wire);
        }
      }
    }

    for (auto wire : del_wires) {
      module->remove({wire});
    }
    del_wires.clear();
  }

  void rem_extra_assigns(Module *module)
  {
    pool<SigBit> assign_bits;
    std::unordered_set<Wire *> del_wires;
    for(auto cell : module->cells())
    {
      for (auto &conn : cell->connections())
      {
        IdString portName = conn.first;
        if (cell->input(portName))
        {
          for (SigBit bit : conn.second)
          {
            if (bit.wire != nullptr) assign_bits.insert(bit);
          }
        }
      }
    }

    for(auto &conn : module->connections())
    {
      for (SigBit bit : conn.second)
      {
        if (bit.wire != nullptr) assign_bits.insert(bit);
      }
    }

    for(auto &conn : module->connections())
    {
      std::vector<RTLIL::SigBit> conn_lhs = conn.first.to_sigbit_vector();
      if (conn_lhs.size() != 1) continue;
      for (SigBit bit : conn.first)
      {
        if (bit.wire != nullptr)
        {
          if(!assign_bits.count(bit) && !bit.wire->port_output)
          {
            connections_to_remove.insert(conn);
            del_wires.insert(bit.wire);
          }
        }
      }
    }

    remove_extra_conns(module);
    connections_to_remove.clear();
    for (auto wire : del_wires) {
      module->remove({wire});
    }
    del_wires.clear();
  }

  void handle_dangling_outs(Module *module)
  {
    for(auto cell : module->cells())
    {
      for (auto &conn : cell->connections())
      {
        IdString portName = conn.first;
        if (cell->input(portName))
        {
          for (SigBit bit : conn.second)
          {
            if (bit.wire != nullptr) used_bits.insert(bit);
          }
        }
      }
    }
    
    while(true)
    {
      unsigned unused_assign = 0;
      pool<SigBit> assign_used_bits;
      for(auto &conn : module->connections())
      {
        for (SigBit bit : conn.second)
        {
          if (bit.wire != nullptr) assign_used_bits.insert(bit);
        }
      }

      for(auto &conn : module->connections())
      {
        std::vector<RTLIL::SigBit> conn_lhs = conn.first.to_sigbit_vector();
        std::vector<RTLIL::SigBit> unused_bits;
        for (SigBit bit : conn.first)
        {
          if (bit.wire != nullptr)
          {
            if(!used_bits.count(bit) && !assign_used_bits.count(bit) && !bit.wire->port_output)
            {
              unused_bits.push_back(bit);
              if(conn.first.is_chunk())
              {
                if(conn.first.as_chunk().width == conn.first.as_chunk().wire->width)
                  del_unused.insert(bit.wire);
              }
            }
          }
        }
        if(unused_bits.size())
        {
          unused_assign++;
          if(unused_bits.size() == conn_lhs.size())
            connections_to_remove.insert(conn);
          else
            std::cerr << "Unused bits in assignement" << std::endl;
        }
      }
      remove_extra_conns(module);
      connections_to_remove.clear();
      for (auto wire : del_unused) {
        module->remove({wire});
      }
      del_unused.clear();
      if (!unused_assign) break;
    }

    for(auto &conn : module->connections())
    {
      for (SigBit bit : conn.second)
      {
        if (bit.wire != nullptr) used_bits.insert(bit);
      }
    }

    for (auto cell : module->cells()){
      string module_name = remove_backslashes(cell->type.str());
      if (std::find(primitives.begin(), primitives.end(), module_name) !=
          primitives.end()) {
        //EDA-3010: output primitives cal also have danlging output wire 
        //bool is_out_prim = (module_name.substr(0, 2) == "O_") ? true : false;
        //if (is_out_prim) continue;
        // Upgrading dangling outs of input primtives to output ports
        for (auto port : cell->connections()){
          IdString portName = port.first;
          for (SigBit bit : port.second){
            if(!used_bits.count(bit) && cell->output(portName)
              && !bit.wire->port_output){
              RTLIL::SigSig new_conn;
              RTLIL::Wire *new_wire = module->addWire(NEW_ID, 1);
              new_wire->port_output = true;
              new_conn.first = new_wire;
              new_conn.second = bit;
              module->connect(new_conn);
            }
          }
        }
      // Upgrading dangling outs of fabric primitives like TDPRAM is causing issues for AES_DECRYPT_partitioner
      //} else {
      //  for (auto port : cell->connections()){
      //    IdString portName = port.first;
      //    for (SigBit bit : port.second){
      //      if(!used_bits.count(bit) && cell->output(portName)
      //       && !bit.wire->port_output){
      //        bit.wire->port_output = true;
      //      }
      //    }
      //  }
      }
    }
  }

  void intersect(const std::unordered_set<std::string>& set1,
    const std::unordered_set<std::string>& set2)
  {
    for (const auto& element : set1)
    {
      if (set2.find(element) != set2.end())
      {
        common_clks_resets.insert(element);
      }
    }
  }

  void intersection_copy_remove(std::unordered_set<std::string> &set1,
    std::unordered_set<std::string> &set2,
    std::unordered_set<std::string> &wires) {
    for (auto it = set1.begin(); it != set1.end();) {
      if (set2.find(*it) != set2.end()) {
        wires.insert(*it);
        it = set1.erase(it);
      } else {
        ++it;
      }
    }
    // Remove elements from set2 that are already moved to wires
    for (auto it = set2.begin(); it != set2.end();) {
      if (wires.find(*it) != wires.end()) {
        it = set2.erase(it);
      } else {
        ++it;
      }
    }
  }

  void handle_inout_connection(Module* mod)
  {
    for(auto &conn : mod->connections())
      {
        std::vector<RTLIL::SigBit> conn_lhs = conn.first.to_sigbit_vector();
        std::vector<RTLIL::SigBit> conn_rhs = conn.second.to_sigbit_vector();
        bool remove_conn = false;
        for (size_t i = 0; i < conn_lhs.size(); ++i)
        {
          if (conn_rhs[i].wire != nullptr)
            if (conn_rhs[i].wire->port_input && conn_rhs[i].wire->port_output)
            {
              inout_conn_map[conn_lhs[i]] = conn_rhs[i];
              remove_conn = true;
            }
        }
        if (remove_conn)
        {
          connections_to_remove.insert(conn);
        }
      }

      remove_extra_conns(mod);
      connections_to_remove.clear();

      for (auto cell : mod->cells())
      {
        for (auto conn : cell->connections())
        {
          IdString portName = conn.first;
          bool unset_port = true;
          RTLIL::SigSpec sigspec;
          for (SigBit bit : conn.second)
          {
            if (inout_conn_map.count(bit) > 0)
            {
              if (unset_port)
              {
                cell->unsetPort(portName);
                unset_port = false;
              }
              sigspec.append(inout_conn_map[bit]);
            } else {
              sigspec.append(bit);
            }
          }
          if (!unset_port) cell->setPort(portName, sigspec);
        }
      }
  }

  void process_wire(Cell *cell, const IdString &portName, RTLIL::Wire *wire) {
    if (cell->input(portName)) {
      if (wire->port_input) {
        inputs.insert(wire->name.str());
      } else {
        new_outs.insert(wire->name.str());
      }
    } else if (cell->output(portName)) {
      if (wire->port_output) {
        outputs.insert(wire->name.str());
      } else {
        new_ins.insert(wire->name.str());
      }
    }
  }

  void add_wire_btw_prims(Module* mod)
  {
    for (const auto& element : out_prim_ins)
    {
      if (in_prim_outs.find(element) != in_prim_outs.end())
      {
        io_prim_wires.insert(element);
      }
    }
    for (const auto& element : io_prim_wires)
    {
      if (new_ins.find(element) != new_ins.end())
      {
        io_prim_wires.insert(element);
      }
    }
    for (const auto& element : io_prim_wires)
    {
      if (new_outs.find(element) != new_outs.end())
      {
        io_prim_wires.insert(element);
      }
    }
    for (const string& element : io_prim_wires) {
      new_ins.erase(element);
      new_outs.erase(element);
    }

    for (auto cell : mod->cells()) {
      string module_name = remove_backslashes(cell->type.str());
      bool is_out_prim = (out_prims.count(module_name) > 0) ? true : false;
      bool is_intf_prim = (soc_intf_prims.count(module_name) > 0) ? true : false;
      if (std::find(primitives.begin(), primitives.end(), module_name) !=
          primitives.end()) {
        for (auto conn : cell->connections()) {
          IdString portName = conn.first;
          bool unset_port = true;
          RTLIL::SigSpec sigspec;
          for (SigBit bit : conn.second)
          {
            if (bit.wire != nullptr)
            {
              if (std::find(io_prim_wires.begin(), io_prim_wires.end(), bit.wire->name.str()) !=
                  io_prim_wires.end()) {
                if (cell->input(portName) &&
                  portName != RTLIL::escape_id("CLK_IN") &&
                  portName != RTLIL::escape_id("C") &&
                  portName != RTLIL::escape_id("PLL_CLK") &&
                  (is_out_prim || is_intf_prim)) {
                  if (unset_port)
                  {
                    cell->unsetPort(portName);
                    unset_port = false;
                  }
                  RTLIL::Wire *new_wire = mod->addWire(NEW_ID, 1);
                  auto it = io_prim_conn.find(bit);

                  if (it != io_prim_conn.end()) {
                    it->second.push_back(new_wire);
                  } else {
                    std::vector<RTLIL::Wire *> new_wires;
                    new_wires.push_back(new_wire);
                    io_prim_conn.insert({bit, new_wires});
                  }
                  new_outs.insert(new_wire->name.str());
                  sigspec.append(new_wire);
                } else if (cell->output(portName)) {
                  if (is_intf_prim)
                  {
                    if (unset_port)
                    {
                      cell->unsetPort(portName);
                      unset_port = false;
                    }
                    RTLIL::Wire *new_wire = mod->addWire(NEW_ID, 1);
                    auto it = intf_prim_conn.find(bit);
                    if (it != intf_prim_conn.end()) {
                      it->second.push_back(new_wire);
                    } else {
                      std::vector<RTLIL::Wire *> new_wires;
                      new_wires.push_back(new_wire);
                      intf_prim_conn.insert({bit, new_wires});
                    }
                    new_ins.insert(new_wire->name.str());
                    sigspec.append(new_wire);
                  } else {
                    new_ins.insert(bit.wire->name.str());
                    keep_wires.insert(bit.wire->name.str());
                    sigspec.append(bit.wire);
                  }
                }
              }
              else {
                sigspec.append(bit);
              }
            }
            else {
              sigspec.append(bit);
            }
          }
          if (!unset_port)
          {
            cell->setPort(portName, sigspec);
          }
        }
      }
    }
  }

  static bool fixup_ports_compare_(const RTLIL::Wire *a, const RTLIL::Wire *b)
  {
    bool has_index_a = a->name.str().back() == ']';
    bool has_index_b = b->name.str().back() == ']';
    if (!has_index_a && !has_index_b)
    {
      return a->name.str() < b->name.str();
    }
    size_t pos_a = has_index_a ? a->name.str().rfind("[") : std::string::npos;
    size_t pos_b = has_index_b ? b->name.str().rfind("[") : std::string::npos;
    std::string prefix_a = pos_a == std::string::npos ? a->name.str() : a->name.str().substr(0, pos_a);
    std::string prefix_b = pos_b == std::string::npos ? b->name.str() : b->name.str().substr(0, pos_b);

    if (prefix_a != prefix_b)
    {
      return prefix_a < prefix_b;
    }

    if (pos_a == std::string::npos || pos_b == std::string::npos)
    {
      return false;
    }

    std::string a_index_str = a->name.str().substr(pos_a + 1,  a->name.str().length() - pos_a - 2);
    std::string b_index_str = b->name.str().substr(pos_b + 1,  b->name.str().length() - pos_b - 2);
    int a_index = std::stoi(a_index_str);
    int b_index = std::stoi(b_index_str);
    return a_index < b_index;
  }

  void fixup_mod_ports (Module* mod)
  {
    std::vector<RTLIL::Wire*> all_ports;

    for (auto w : mod->wires())
    {
      if (w->port_input || w->port_output)
        all_ports.push_back(w);
      else
        w->port_id = 0;
    }

    std::sort(all_ports.begin(), all_ports.end(), fixup_ports_compare_);
    mod->ports.clear();
    for (size_t i = 0; i < all_ports.size(); i++) 
    {
      mod->ports.push_back(all_ports[i]->name);
      all_ports[i]->port_id = i+1;
    }
  }

  void remove_extra_conns(Module* mod)
  {
    for (const auto& conn : connections_to_remove) {
    mod->connections_.erase(std::remove_if(mod->connections_.begin(),
      mod->connections_.end(),
      [&](const std::pair<Yosys::RTLIL::SigSpec, Yosys::RTLIL::SigSpec>& p) {
          return p == conn;
      }), mod->connections_.end());
    }
  }

  bool is_clk_out(Module *mod, Wire* rhs_wire, std::unordered_set<std::string> &prims)
  {
    bool is_clk_output = false;
    for (auto cell : mod->cells()) {
      string module_name = remove_backslashes(cell->type.str());
      if (std::find(prims.begin(), prims.end(), module_name) !=
          prims.end()) {
        for (auto conn : cell->connections()) {
          IdString portName = conn.first;
          RTLIL::SigSpec actual = conn.second;
          if (actual.is_chunk()) {
            const RTLIL::SigChunk chunk = actual.as_chunk();
            if(chunk.wire == NULL) continue;
            if(chunk.wire->name.str() == rhs_wire->name.str() &&
              (module_name.substr(0, 4) == "CLK_"))
            {
              is_clk_output = true;
            }
          }
        }
      }
    }
    return is_clk_output;
  }

  void update_prim_connections(Module* mod, std::unordered_set<std::string> &prims, std::unordered_set<Wire *> &del_intermediate_wires)
  {
    for (auto cell : mod->cells()) {
      string module_name = remove_backslashes(cell->type.str());
      if (std::find(prims.begin(), prims.end(), module_name) !=
          prims.end()) {
        for (auto conn : cell->connections()) {
          IdString portName = conn.first;
          RTLIL::SigSpec actual = conn.second;
          if (actual.is_chunk()) {
            const RTLIL::SigChunk chunk = actual.as_chunk();
            RTLIL::Wire *wire = actual.as_chunk().wire;
            if(chunk.wire == NULL) continue;
            for (const auto& connection : connections_to_remove)
            {
              const Yosys::RTLIL::SigSpec lhs = connection.first;
              const Yosys::RTLIL::SigSpec rhs = connection.second;
              const RTLIL::SigChunk lhs_chunk = lhs.as_chunk();
              const RTLIL::SigChunk rhs_chunk = rhs.as_chunk();
              if ((chunk.width == chunk.wire->width && chunk.offset == 0) &&
                (lhs_chunk.width == lhs_chunk.wire->width && lhs_chunk.offset == 0) &&
                (lhs_chunk.wire->name.str() == chunk.wire->name.str()))
              {
                cell->unsetPort(portName);
                cell->setPort(portName, rhs);
                del_intermediate_wires.insert(wire);
              } else if ((chunk.width == 1) &&
                (lhs_chunk.wire->name.str() == chunk.wire->name.str()))
              {
                if (lhs_chunk.width == 1)
                {
                  cell->unsetPort(portName);
                  cell->setPort(portName, rhs);
                  del_intermediate_wires.insert(wire);
                } else if (lhs_chunk.width == lhs_chunk.wire->width && lhs_chunk.offset == 0) {
                  unsigned offset = chunk.offset + chunk.wire->start_offset ;
                  auto conn_rhs = connection.second.to_sigbit_vector();
                  cell->unsetPort(portName);
                  cell->setPort(portName, conn_rhs.at(offset));
                  del_intermediate_wires.insert(wire);
                }
              }
            }
          }
        }
      }
    }
  }

  void clean_flattened(Module *mod)
  {
    for(auto &conn : mod->connections())
    {
      std::vector<RTLIL::SigBit> conn_lhs = conn.first.to_sigbit_vector();
      std::vector<RTLIL::SigBit> conn_rhs = conn.second.to_sigbit_vector();
      for (size_t i = 0; i < conn_lhs.size(); i++) {
        if (conn_lhs[i].wire != nullptr && conn_rhs[i].wire != nullptr)
        {
          wrapper_conns.insert(std::make_pair(conn_lhs[i], conn_rhs[i]));
        } else {
          std::cerr << "Unexpected behaviour from flatten pass" << std::endl;
        }
      }
    }

    for (auto cell : mod->cells()) {
      string module_name = cell->type.str();
      bool is_fabric_instance = (module_name.substr(0, 8) == "\\fabric_") ? true : false;
      if (is_fabric_instance) continue;
      for (auto conn : cell->connections()) {
        IdString portName = conn.first;
        bool unset_port = true;
        RTLIL::SigSpec sigspec;
        for (SigBit bit : conn.second)
        {
          if (bit.wire != nullptr)
          {
            bool appended = false;
            for (auto it = wrapper_conns.begin(); it != wrapper_conns.end(); ++it) {
              if (it->second == bit) {
                if (unset_port) {
                  cell->unsetPort(portName);
                  unset_port = false;
                }
                sigspec.append(it->first);
                appended = true;
                break;
              } else if (it->first == bit) {
                if (unset_port) {
                  cell->unsetPort(portName);
                  unset_port = false;
                }
                sigspec.append(it->second);
                appended = true;
                break;
              }
            }
            if(!appended) sigspec.append(bit);
          }
          else {
            sigspec.append(bit);
          }
        }
        if (!unset_port)
        {
          cell->setPort(portName, sigspec);
        }
      }
    }

    mod->connections_.clear();
  }

  void elapsed_time (time_point<high_resolution_clock> start,
    time_point<high_resolution_clock> end)
  {
    auto duration = duration_cast<nanoseconds>(end - start);
    float totalTime = duration.count() * 1e-9;
    std::cout << "[" << totalTime << " sec.]\n";
  }

  void get_fabric_ios(Module* mod)
  {
    for (auto wire : mod->wires())
    {
      bool is_output = wire->port_output ? true :false;
      bool is_input = wire->port_input ? true :false;
      if (!is_output && !is_input) continue;

      if (is_input)
      {
        RTLIL::SigSpec wire_ = wire;
        for (auto bit : wire_) fab_ins.insert(bit);
      }

      if (is_output)
      {
        RTLIL::SigSpec wire_ = wire;
        for (auto bit : wire_) fab_outs.insert(bit);
      }
    }
  }

  static bool sigName(const RTLIL::SigSpec &sig, std::string &name)
  {
    if (!sig.is_chunk())
    {
      return false;
    }

    const RTLIL::SigChunk chunk = sig.as_chunk();

    if (chunk.wire == NULL)
    {
      return false;
    }

    if (chunk.width == chunk.wire->width && chunk.offset == 0)
    {
      name = (chunk.wire->name).substr(0);
    }
    else
    {
      name = "";
    }

    return true;
  }

  static int checkCell(Cell *cell, const string cellName,
                const string &port, string &actual_name)
  {
    if (cell->type != RTLIL::escape_id(cellName))
    {
      return 0;
    }

    std::string name;
    for (auto &conn : cell->connections())
    {

      IdString portName = conn.first;
      RTLIL::SigSpec actual = conn.second;

      if (portName == RTLIL::escape_id(port))
      {
        if (sigName(actual, name))
        {
          actual_name = name;
          if (actual_name[0] == '\\') {
            actual_name = actual_name.substr(1);
          }
          return 1;
        }
      }
    }
    return 1;
  }

  bool is_flag(const std::string &arg) { return !arg.empty() && arg[0] == '-'; }

  std::string get_extension(const std::string &filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
      return filename.substr(dot_pos);
    }
    return ""; // If no extension found
  }

 static void reportInfoFabricClocks(Module *original_mod) {
  std::ofstream fabric_clocks("fabric_netlist_info.json");
    json ports = json::object();
    json ports_array = json::array();
    std::set<std::string> reported;
    for (auto cell : original_mod->cells())
    {
      string module_name = cell->type.str();
      string actual_clock;
      if (checkCell(cell, "DFFRE",
                    "C", actual_clock))
      {
        if (reported.find(actual_clock) == reported.end())
        {
          json port_object;
          port_object["name"] = actual_clock;
          port_object["direction"] = (std::string)("input");
          port_object["clock"] = (std::string)("active_high");
          ports_array.push_back(port_object);
          reported.insert(actual_clock);
        }
        continue;
      }

      if (checkCell(cell, "DFFNRE",
                    "C", actual_clock))
      {
        if (reported.find(actual_clock) == reported.end())
        {
          json port_object;
          port_object["name"] = actual_clock;
          port_object["direction"] = (std::string)("input");
          port_object["clock"] = (std::string)("active_low");
          ports_array.push_back(port_object);
          reported.insert(actual_clock);
        }
        continue;
      }
      if (checkCell(cell, "DSP38",
                    "CLK", actual_clock) || checkCell(cell, "DSP19x2",
                    "CLK", actual_clock))
      {
        if (reported.find(actual_clock) == reported.end())
        {
          json port_object;
          port_object["name"] = actual_clock;
          port_object["direction"] = (std::string)("input");
          port_object["clock"] = (std::string)("active_high");
          ports_array.push_back(port_object);
          reported.insert(actual_clock);
        }
        continue;
      }
      for (auto formal_clock : {"CLK_A", "CLK_B"})
      {
        if (checkCell(cell, "TDP_RAM36K",
                      formal_clock, actual_clock))
        {
          if (reported.find(actual_clock) == reported.end())
          {
            json port_object;
            port_object["name"] = actual_clock;
            port_object["direction"] = (std::string)("input");
            port_object["clock"] = (std::string)("active_high");
            ports_array.push_back(port_object);
            reported.insert(actual_clock);
          }
        }
      }
      for (auto formal_clock : {"CLK_A1", "CLK_B1", "CLK_A2", "CLK_B2"})
      {
        if (checkCell(cell, "TDP_RAM18KX2",
                      formal_clock, actual_clock))
        {
          if (reported.find(actual_clock) == reported.end())
          {
            json port_object;
            port_object["name"] = actual_clock;
            port_object["direction"] = (std::string)("input");
            port_object["clock"] = (std::string)("active_high");
            ports_array.push_back(port_object);
            reported.insert(actual_clock);
          }
        }
      }
    }
    ports["ports"] = ports_array;
    if (fabric_clocks.is_open()) {
      fabric_clocks << std::setw(4) << ports << std::endl;
      fabric_clocks.close();
    }
  }



  void execute(std::vector<std::string> args, RTLIL::Design *design) override {
    std::string run_from, run_to;
    clear_flags();
    _design = design;

    size_t argidx;
    // TODO: Will send the arguments and test after parsing is done
    for (argidx = 1; argidx < args.size(); argidx++) {
      if (args[argidx] == "-w" && argidx + 1 < args.size()) {
        size_t next_argidx = argidx + 1;
        while (next_argidx < args.size() && !is_flag(args[next_argidx])) {
          wrapper_files.push_back(args[next_argidx]);
          ++next_argidx;
        }
        argidx = next_argidx - 1;
        continue;
      }
      if (args[argidx] == "-pr" && argidx + 1 < args.size()) {
        size_t next_argidx = argidx + 1;
        while (next_argidx < args.size() && !is_flag(args[next_argidx])) {
          post_route_wrapper.push_back(args[next_argidx]);
          ++next_argidx;
        }
        argidx = next_argidx - 1;
        continue;
      }
      if (args[argidx] == "-tech" && argidx + 1 < args.size())
      {
        tech = args[++argidx];
        continue;
      }
      if (args[argidx] == "-json" && argidx + 1 < args.size())
      {
        io_config_json = args[++argidx];
        continue;
      }
      if (args[argidx] == "-sdc" && argidx + 1 < args.size())
      {
        sdc_file = args[++argidx];
        sdc_passed = true;
        continue;
      }
      break;
    }
    primitives = io_prim.get_primitives(tech);
    categorize_primitives();
    bool supported_tech = io_prim.supported_tech;

    auto start = high_resolution_clock::now();
    auto start_time = start;
    NETLIST_CHECKER checker;
    checker.prims = primitives;
    log("Extracting primitives\n");
    // Extract the primitive information (before anything is modified)
    PRIMITIVES_EXTRACTOR* extractor = new PRIMITIVES_EXTRACTOR(tech);
    extractor->extract(_design);
    auto end = high_resolution_clock::now();
    elapsed_time (start, end);
    
    if (sdc_passed) {
      std::ifstream input_sdc(sdc_file);
      if (!input_sdc.is_open()) {
        std::cerr << "Error opening input sdc file: " << sdc_file << std::endl;
      }
      processSdcFile(input_sdc);
      for (auto &p : pins) {
        extractor->assign_location(p->_name, p->_location, p->_properties);
      }
    }

    start = high_resolution_clock::now();
    log("Running SplitNets\n");
    Pass::call(_design, "splitnets");
    end = high_resolution_clock::now();
    elapsed_time (start, end);
    Module *original_mod = _design->top_module();
    std::string original_mod_name =
      remove_backslashes(_design->top_module()->name.str());
    design->rename(original_mod, "\\fabric_" + original_mod_name);

    for (auto wire : original_mod->wires())
    {
      bool is_input = wire->port_input ? true :false;
      bool is_output = wire->port_output ? true :false;
      if (!is_input && !is_output) continue;

      RTLIL::SigSpec wire_ = wire;
      for (auto bit : wire_)
      {
        if (is_input) orig_ins.insert(bit);
        if (is_output) orig_outs.insert(bit);
      }
    }
    checker.design_inputs = orig_ins;
    checker.design_outputs = orig_outs;

    start = high_resolution_clock::now();
    log("Gathering Wires Data\n");
    if (supported_tech)
    {
      for (auto cell : original_mod->cells()) {
        string module_name = remove_backslashes(cell->type.str());
        if (std::find(primitives.begin(), primitives.end(), module_name) !=
            primitives.end()) {
          io_prim.contains_io_prem = true;
          bool is_out_prim = (out_prims.count(module_name) > 0) ? true : false;
          bool is_intf_prim = (soc_intf_prims.count(module_name) > 0) ? true : false;
          remove_prims.push_back(cell);

          for (auto conn : cell->connections()) {
            IdString portName = conn.first;
            RTLIL::SigSpec actual = conn.second;
            bool unset_port = true;
            RTLIL::SigSpec sigspec;
            if (actual.is_chunk()) {
              RTLIL::Wire *wire = actual.as_chunk().wire;
              if (wire != NULL) {
                process_wire(cell, portName, wire);
                if (is_out_prim || is_intf_prim) {
                  if (cell->input(portName)) {
                    if (portName != RTLIL::escape_id("CLK_IN") &&
                      portName != RTLIL::escape_id("C") &&
                      portName != RTLIL::escape_id("PLL_CLK"))
                      out_prim_ins.insert(wire->name.str());
                  }
                }
                if (!is_out_prim) {
                  if (cell->output(portName)) {
                    in_prim_outs.insert(wire->name.str());
                    for (auto bit : conn.second){
                      prim_out_bits.insert(bit);
                    }
                  }
                }
              }
            } else {
              for (auto it = actual.chunks().rbegin();
                   it != actual.chunks().rend(); ++it) {
                RTLIL::Wire *wire = (*it).wire;
                if (wire != NULL) {
                  process_wire(cell, portName, wire);
                  if (is_out_prim || is_intf_prim) {
                    if (cell->input(portName)) {
                      if (portName != RTLIL::escape_id("CLK_IN") &&
                      portName != RTLIL::escape_id("C") &&
                      portName != RTLIL::escape_id("PLL_CLK"))
                        out_prim_ins.insert(wire->name.str());
                    }
                  } 
                  if (!is_out_prim) {
                    if (cell->output(portName)) {
                      in_prim_outs.insert(wire->name.str());
                      for (auto bit : conn.second){
                        prim_out_bits.insert(bit);
                      }
                    }
                  }
                }
              }
            }
            for (SigBit bit : conn.second)
            {
              // Route constant bits through fabric
              if (bit.wire == nullptr)
              {
                if (unset_port)
                {
                  cell->unsetPort(portName);
                  unset_port = false;
                }
                RTLIL::SigSig new_conn;
                RTLIL::Wire *new_wire = original_mod->addWire(NEW_ID, 1);
                new_conn.first = new_wire;
                new_conn.second = bit;
                original_mod->connect(new_conn);
                new_outs.insert(new_wire->name.str());
                sigspec.append(new_wire);
              } else {
                sigspec.append(bit);
              }
            }

            if (!unset_port)
            {
              cell->setPort(portName, sigspec);
            }
          }
        } else {
          for (auto conn : cell->connections()) {
            IdString portName = conn.first;
            RTLIL::SigSpec actual = conn.second;
            if (actual.is_chunk()) {
              RTLIL::Wire *wire = actual.as_chunk().wire;
              if (wire != NULL) {
                keep_wires.insert(wire->name.str());
              }
            } else {
              for (auto it = actual.chunks().rbegin();
                   it != actual.chunks().rend(); ++it) {
                RTLIL::Wire *wire = (*it).wire;
                if (wire != NULL) {
                  keep_wires.insert(wire->name.str());
                }
              }
            }
          }
        }
      }

      checker.gather_bufs_data(original_mod);
      checker.check_buf_conns();
      checker.check_clkbuf_conns();
      checker.check_idly_data_ins();
      checker.check_odly_data_outs();
      checker.check_iserdes_data_ins();
      checker.check_oserdes_data_outs();
      checker.check_iddr_data_ins();
      checker.check_oddr_data_outs();
      end = high_resolution_clock::now();
      elapsed_time (start, end);

      start = high_resolution_clock::now();
      log("Adding wires between directly connected input and output primitives\n");
      add_wire_btw_prims(original_mod);
      end = high_resolution_clock::now();
      elapsed_time (start, end);
      intersection_copy_remove(new_ins, new_outs, interface_wires);
      intersect(interface_wires, keep_wires);
    }
    
    Module *interface_mod = _design->top_module()->clone();
    std::string interface_mod_name = "\\interface_" + original_mod_name;
    interface_mod->name = interface_mod_name;
    Module *wrapper_mod = original_mod->clone();
    std::string wrapper_mod_name = "\\" + original_mod_name;
    wrapper_mod->name = wrapper_mod_name;

    if (supported_tech)
    {
      start = high_resolution_clock::now();
      log("Upgrading fabric wires to ports\n");
      for (auto wire : original_mod->wires()) {
        std::string wire_name = wire->name.str();
        if (new_ins.find(wire_name) != new_ins.end()) {
          wire->port_input = true;
          checker.fab_ins.insert(wire);
          continue;
        }
        if (new_outs.find(wire_name) != new_outs.end()) {
          wire->port_output = true;
          checker.fab_outs.insert(wire);
          continue;
        }
        if (common_clks_resets.find(wire_name) != common_clks_resets.end())
        {
          wire->port_input = true;
          checker.fab_ins.insert(wire);
          continue;
        }
        if (interface_wires.find(wire_name) != interface_wires.end()) {
          wires_interface.insert(wire);
          continue;
        }
        if (inputs.find(wire_name) != inputs.end()) {
          del_ins.insert(wire);
          continue;
        }
        if (outputs.find(wire_name) != outputs.end()) {
          del_outs.insert(wire);
          continue;
        }
      }
      end = high_resolution_clock::now();
      elapsed_time (start, end);

      start = high_resolution_clock::now();
      log("Handling I_BUF->Fabric->CLK_BUF\n");
      for (auto &conn : original_mod->connections()) {
        RTLIL::SigSpec lhs = conn.first;
        RTLIL::SigSpec rhs = conn.second;
        if(lhs.is_chunk() && rhs.is_chunk())
        {
          const RTLIL::SigChunk lhs_chunk = lhs.as_chunk();
          const RTLIL::SigChunk rhs_chunk = rhs.as_chunk();
          if((lhs_chunk.wire != nullptr) && (rhs_chunk.wire != nullptr))
          {
            if((lhs_chunk.wire->port_input || lhs_chunk.wire->port_output) &&
              (rhs_chunk.wire->port_input || rhs_chunk.wire->port_output) &&
              (outputs.find(lhs_chunk.wire->name.str()) == outputs.end()))
            {
              if(is_clk_out(original_mod, rhs_chunk.wire, primitives) &&
                inputs.find(rhs_chunk.wire->name.str()) == inputs.end())
              {
                lhs_chunk.wire->port_input = false;
                lhs_chunk.wire->port_output = false;
                rhs_chunk.wire->port_input = false;
                rhs_chunk.wire->port_output = false;
                connections_to_remove.insert(conn);
              }
            }
          }
        }
      }

      remove_extra_conns(original_mod);
      connections_to_remove.clear();
      update_prim_connections(original_mod, primitives, orig_intermediate_wires);
      end = high_resolution_clock::now();
      elapsed_time (start, end);

      for (const auto& prim_conn : io_prim_conn) {
        const std::vector<RTLIL::Wire *>& connected_wires = prim_conn.second;
        if(connected_wires.size() < 1) continue;
        RTLIL::SigSpec in_prim_out;
        pool<RTLIL::SigSpec> out_prim_in;
        for(const auto conn_wire : connected_wires) {
          std::string wire_name = conn_wire->name.str();
          out_prim_in.insert(conn_wire);
        }
        for(const auto& prim_in : out_prim_in)
        {
          RTLIL::SigSig new_conn;
          new_conn.first = prim_in;
          new_conn.second = prim_conn.first;
          original_mod->connect(new_conn);
        }
      }

      for (const auto& prim_conn : intf_prim_conn) {
        const std::vector<RTLIL::Wire *>& connected_wires = prim_conn.second;
        if(connected_wires.size() < 1) continue;
        pool<RTLIL::SigSpec> in_prim_out;
        for(const auto conn_wire : connected_wires) {
          std::string wire_name = conn_wire->name.str();
          in_prim_out.insert(conn_wire);
        }
        for(const auto& prim_in : in_prim_out)
        {
          RTLIL::SigSig new_conn;
          new_conn.first = prim_conn.first;
          new_conn.second = prim_in;
          original_mod->connect(new_conn);
        }
      }

      start = high_resolution_clock::now();
      log("Handling Dangling outs\n");
      handle_dangling_outs(original_mod);
      end = high_resolution_clock::now();
      elapsed_time (start, end);
      checker.gather_prims_data(original_mod);
      checker.check_buf_cntrls();
      checker.check_fclkbuf_conns();
      checker.check_odly_data_ins();
      checker.check_idly_data_outs();
      checker.check_dly_cntrls();
      checker.check_ddr_cntrls();
      checker.check_iddr_data_outs();
      checker.check_oddr_data_ins();
      checker.check_iserdes_data_outs();
      checker.check_oserdes_data_ins();
      checker.check_serdes_cntrls();
      start = high_resolution_clock::now();
      log("Deleting primitive cells and extra wires\n");
      delete_cells(original_mod, remove_prims);

      for (auto &conn : original_mod->connections()) {
        std::vector<RTLIL::SigBit> conn_lhs = conn.first.to_sigbit_vector();
        std::vector<RTLIL::SigBit> conn_rhs = conn.second.to_sigbit_vector();
        for (size_t i = 0; i < conn_lhs.size(); i++) {
          if (conn_lhs[i].wire != nullptr) {
            keep_wires.insert(conn_lhs[i].wire->name.str());
          }
          if (conn_rhs[i].wire != nullptr) {
            keep_wires.insert(conn_rhs[i].wire->name.str());
          }
        }
      }

      delete_wires(original_mod, wires_interface);
      delete_wires(original_mod, del_ins);
      delete_wires(original_mod, del_outs);
      end = high_resolution_clock::now();
      elapsed_time (start, end);

      fixup_mod_ports(original_mod);

      get_fabric_ios(original_mod);

      remove_io_fab_prim(original_mod);

      start = high_resolution_clock::now();
      log("Deleting non-primitive cells and upgrading wires to ports in interface module\n");
      for (auto cell : interface_mod->cells()) {
        string module_name = remove_backslashes(cell->type.str());
        if (std::find(primitives.begin(), primitives.end(), module_name) ==
            primitives.end()) {
          remove_non_prims.push_back(cell);
        }
      }

      delete_cells(interface_mod, remove_non_prims);

      for (auto wire : interface_mod->wires()) {
        std::string wire_name = wire->name.str();
        if (new_ins.find(wire_name) != new_ins.end()) {
          wire->port_output = true;
          continue;
        }
        if (new_outs.find(wire_name) != new_outs.end()) {
          wire->port_input = true;
          continue;
        }
        if (common_clks_resets.find(wire_name) != common_clks_resets.end())
        {
          wire->port_output = true;
          continue;
        }
        if (interface_wires.find(wire_name) != interface_wires.end()) {
          continue;
        }
        if (inputs.find(wire_name) != inputs.end()) {
          continue;
        }
        if (outputs.find(wire_name) != outputs.end()) {
          continue;
        }
        del_interface_wires.insert(wire);
      }

      end = high_resolution_clock::now();
      elapsed_time (start, end);

      start = high_resolution_clock::now();
      log("Handling I_BUF->Fabric->CLK_BUF in interface module\n");
      for (auto &conn : interface_mod->connections()) {
        RTLIL::SigSpec lhs = conn.first;
        RTLIL::SigSpec rhs = conn.second;
        if(lhs.is_chunk() && rhs.is_chunk())
        {
          const RTLIL::SigChunk lhs_chunk = lhs.as_chunk();
          const RTLIL::SigChunk rhs_chunk = rhs.as_chunk();
          if((lhs_chunk.wire != nullptr) && (rhs_chunk.wire != nullptr))
          {
            if((lhs_chunk.wire->port_input || lhs_chunk.wire->port_output) &&
              (rhs_chunk.wire->port_input || rhs_chunk.wire->port_output) &&
              (outputs.find(lhs_chunk.wire->name.str()) == outputs.end()))
            {
              if(is_clk_out(interface_mod, lhs_chunk.wire, primitives) &&
                inputs.find(rhs_chunk.wire->name.str()) == inputs.end())
              {
                lhs_chunk.wire->port_input = false;
                lhs_chunk.wire->port_output = false;
                rhs_chunk.wire->port_input = false;
                rhs_chunk.wire->port_output = false;
                connections_to_remove.insert(conn);
              }
            }
          }
        }
      }

      update_prim_connections(interface_mod, primitives, interface_intermediate_wires);
      end = high_resolution_clock::now();
      elapsed_time (start, end);

      interface_mod->connections_.clear();
      connections_to_remove.clear();
      start = high_resolution_clock::now();
      log("Removing extra wires from interface module\n");
      for (auto wire : del_interface_wires) {
        interface_mod->remove({wire});
      }
      end = high_resolution_clock::now();
      elapsed_time (start, end);

      delete_wires(original_mod, orig_intermediate_wires);
      fixup_mod_ports(original_mod);
      start = high_resolution_clock::now();
      log("Cleaning fabric netlist\n");
      Pass::call(_design, "clean");
      end = high_resolution_clock::now();
      elapsed_time (start, end);

      rem_extra_assigns(original_mod);
      rem_extra_wires(original_mod);

      reportInfoFabricClocks(original_mod);

      delete_wires(interface_mod, interface_intermediate_wires);
      interface_mod->fixup_ports();
    }

    start = high_resolution_clock::now();
    log("Removing cells from wrapper module\n");
    for (auto cell : wrapper_mod->cells()) {
      string module_name = cell->type.str();
      remove_wrapper_cells.push_back(cell);
    }

    for (auto cell : remove_wrapper_cells) {
      wrapper_mod->remove(cell);
    }
    end = high_resolution_clock::now();
    elapsed_time (start, end);

    wrapper_mod->connections_.clear();

    start = high_resolution_clock::now();
    log("Instantiating fabric and interface modules\n");
    // Add instances of the original and interface modules to the wrapper module
    Cell *orig_mod_inst = wrapper_mod->addCell("\\fabric_instance", original_mod->name);
    for (auto wire : original_mod->wires()) {
      RTLIL::SigSpec conn = wire;
      std::string wire_name = wire->name.str();
      if (wire->port_input || wire->port_output) {
        orig_inst_conns.insert(wire_name);
      }
    }

    for (auto wire : interface_mod->wires()) {
      RTLIL::SigSpec conn = wire;
      std::string wire_name = wire->name.str();
      if (wire->port_input || wire->port_output) {
        interface_inst_conns.insert(wire_name);
      }
    }

    if (supported_tech)
    {
      Cell *interface_mod_inst =
        wrapper_mod->addCell(NEW_ID, interface_mod->name);
      for (auto wire : wrapper_mod->wires()) {
        RTLIL::SigSpec conn = wire;
        std::string wire_name = wire->name.str();
        if (orig_inst_conns.find(wire_name) == orig_inst_conns.end() &&
          interface_inst_conns.find(wire_name) == interface_inst_conns.end() &&
          interface_wires.find(wire_name) == interface_wires.end()) {
          del_wrapper_wires.insert(wire);
        } else {
          if (orig_inst_conns.find(wire_name) != orig_inst_conns.end()) {
            orig_mod_inst->setPort(wire_name, conn);
          }
          if (supported_tech)
          {
            if (interface_inst_conns.find(wire_name) !=
              interface_inst_conns.end()) {
            interface_mod_inst->setPort(wire_name, conn);
            }
          }
        }
      }
    } else {
      for (auto wire : wrapper_mod->wires()) {
        RTLIL::SigSpec conn = wire;
        std::string wire_name = wire->name.str();
        if (orig_inst_conns.find(wire_name) == orig_inst_conns.end()) {
          del_wrapper_wires.insert(wire);
        } else {
          orig_mod_inst->setPort(wire_name, conn);
        }
      }
    }
    end = high_resolution_clock::now();
    elapsed_time (start, end);

    start = high_resolution_clock::now();
    log("Removing extra wires from wrapper module\n");
    for (auto wire : del_wrapper_wires) {
      wrapper_mod->remove({wire});
    }
    end = high_resolution_clock::now();
    elapsed_time (start, end);

    start = high_resolution_clock::now();
    log("Fixing wrapper ports\n");
    wrapper_mod->fixup_ports();

    new_design->add(wrapper_mod);
    if (supported_tech)
    {
      new_design->add(interface_mod);
    }
    end = high_resolution_clock::now();
    elapsed_time (start, end);
    start = high_resolution_clock::now();
    log("Flattening wrapper module\n");
    Pass::call(new_design, "flatten");
    end = high_resolution_clock::now();
    elapsed_time (start, end);
    handle_inout_connection(wrapper_mod);

    start = high_resolution_clock::now();
    log("Removing extra assigns from wrapper module\n");
    clean_flattened(wrapper_mod);
    end = high_resolution_clock::now();
    elapsed_time (start, end);

    for (auto file : wrapper_files) {
      std::string extension = get_extension(file);
      if (!extension.empty()) {
        if (extension == ".v") {
          Pass::call(new_design, "write_verilog -noexpr -norename " + file);
          continue;
        }
        if (extension == ".eblif") {
          Pass::call(new_design, "write_blif -param " + file);
          continue;
        }
      }
    }

    for(auto cell : wrapper_mod->cells())
    {
      if(cell->type.str() == orig_mod_inst->type.str())
      {
        for(const auto& conn : cell->connections())
        {
          RTLIL::SigSpec actual = conn.second;
          IdString portName = conn.first;
          if (actual.is_chunk())
          {
            const RTLIL::SigChunk chunk = actual.as_chunk();
            RTLIL::Wire *wire = actual.as_chunk().wire;
            if(chunk.wire == NULL) continue;
            if(wire->width > 1)
            {
              cell->unsetPort(portName);
              RTLIL::SigSpec conn = wire;
              int width = wire->width;
              for(int i=0; i<width; i++)
              {
                IdString nportName = std::string(portName.c_str()) + "[" + std::to_string(i) + "]";
                unsigned index = (wire->upto) ? (width - 1 - i) : i;
                cell->setPort(nportName, conn[index]);
              }
            }
          }
        }
      }
    }

    run_script(new_design);
    checker.write_checker_file();
    if (supported_tech)
    {
      start = high_resolution_clock::now();
      log("Dumping config.json\n");
      // Dump entire wrap design using "config.json" naming (by default)
      dump_io_config_json(wrapper_mod, io_config_json);
      end = high_resolution_clock::now();
      elapsed_time (start, end);
      start = high_resolution_clock::now();
      log("Updating sdc\n");
      std::ifstream input(io_config_json.c_str());
      log_assert(input.is_open() && input.good());
      nlohmann::json instances = nlohmann::json::parse(input);
      input.close();
      log_assert(instances.is_object());
      log_assert(instances.contains("instances"));
      extractor->write_sdc("design_edit.sdc", "clk_pin.xml", instances["instances"]);
      std::string io_file = "io_" + io_config_json;
      extractor->write_json(io_file);
      end = high_resolution_clock::now();
      elapsed_time (start, end);
      auto end_time = end;
      auto duration = duration_cast<nanoseconds>(end_time - start_time);
      float totalTime = duration.count() * 1e-9;
      std::cout << "Time elapsed in design editing : " << " [" << totalTime << " sec.]\n";
    }
    delete extractor;
    if (checker.netlist_error)
      log_error("Netlist is illegal, check netlist_checker.log for more details.\n");
  }

  void script() override {
    std::cout << "Run Script" << std::endl;
    for (auto file : post_route_wrapper) {
      std::string extension = get_extension(file);
      if (!extension.empty()) {
        if (extension == ".v") {
          run("write_verilog -noexpr -norename " + file);
          continue;
        }
        if (extension == ".eblif") {
          run("write_blif -param " + file);
          continue;
        }
      }
    }
  }
} DesignEditRapidSilicon;

PRIVATE_NAMESPACE_END
