#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <cstdint>

#include "Store.h"
#include "SlowControlElement.h"
#include "SlowControlCollection.h"
#include "Services.h"
#include "DAQInterface.h"

namespace py = pybind11;
using namespace ToolFramework;

template<typename T>
void bind_get(py::class_<Store>& cls, const char* name) {
    cls.def(name, [](Store &self, std::string key) -> py::object {
        T val;
        if (self.Get(key, val)) return py::cast(val);
        return py::none();
    });
}

template<typename T>
void bind_get_vector(py::class_<Store>& cls, const char* name) {
    cls.def(name, [](Store &self, std::string key) -> py::object {
        std::vector<T> val;
        if (self.Get(key, val)) return py::cast(val);
        return py::none();
    });
}

template<typename T>
void bind_set(py::class_<Store>& cls, const char* name) {
    cls.def(name, [](Store &self, std::string key, T val) {
        self.Set(key, val);
    });
}

template<typename T>
void bind_set_vector(py::class_<Store>& cls, const char* name) {
    cls.def(name, [](Store &self, std::string key, std::vector<T> val) {
        self.Set(key, val);
    });
}

// ---------------------------------------------------------------------
// Binding class Store
// ---------------------------------------------------------------------

void init_store(py::module_ &tf) {
    auto store_cls = py::class_<Store>(tf, "Store");

    store_cls
        .def(py::init<>())
        .def("Initialise", &Store::Initialise, py::arg("filename"))
        .def("JsonParser", &Store::JsonParser, py::arg("input"))
        .def("Print", &Store::Print)
        .def("Delete", &Store::Delete)
        .def("Has", &Store::Has, py::arg("key"))
        .def("Keys", &Store::Keys)
        .def("Destring", &Store::Destring, py::arg("key"))
        .def("Erase", &Store::Erase, py::arg("key"))

        .def("GetString", [](Store &self, std::string key) -> py::object {
            std::string val;
            if (self.Get(key, val)) return py::cast(val);
            return py::none();
        })

        .def("GetBool", [](Store &self, std::string key) -> py::object {
            bool val;
            if (self.Get(key, val)) return py::cast(val);
            return py::none();
        })

        .def("GetStore", [](Store &self, std::string key) -> py::object {
            Store val;
            if (self.Get(key, val)) return py::cast(val);
            return py::none();
        })

        .def("SetString", [](Store &self, std::string key, std::string val) {
            self.Set(key, val);
        })

        .def("ToJson", [](Store &self) {
            std::string out;
            self >> out;
            return out;
        })

        .def("__repr__", [](Store &self) {
            std::string out;
            self >> out;
            return out;
        })

        .def("__contains__", &Store::Has)
        ;

    bind_get<int>(store_cls, "GetInt");
    bind_get<long>(store_cls, "GetLong");
    bind_get<uint64_t>(store_cls, "GetUInt64");
    bind_get<double>(store_cls, "GetDouble");
    bind_get<float>(store_cls, "GetFloat");

    bind_get_vector<int>(store_cls, "GetIntVector");
    bind_get_vector<long>(store_cls, "GetLongVector");
    bind_get_vector<uint64_t>(store_cls, "GetUInt64Vector");
    bind_get_vector<double>(store_cls, "GetDoubleVector");
    bind_get_vector<float>(store_cls, "GetFloatVector");
    bind_get_vector<std::string>(store_cls, "GetStringVector");

    bind_set<int>(store_cls, "SetInt");
    bind_set<long>(store_cls, "SetLong");
    bind_set<uint64_t>(store_cls, "SetUInt64");
    bind_set<double>(store_cls, "SetDouble");
    bind_set<float>(store_cls, "SetFloat");
    bind_set<bool>(store_cls, "SetBool");

    bind_set_vector<int>(store_cls, "SetIntVector");
    bind_set_vector<long>(store_cls, "SetLongVector");
    bind_set_vector<uint64_t>(store_cls, "SetUInt64Vector");
    bind_set_vector<double>(store_cls, "SetDoubleVector");
    bind_set_vector<float>(store_cls, "SetFloatVector");
    bind_set_vector<std::string>(store_cls, "SetStringVector");
}

// ---------------------------------------------------------------------
// Binding class SlowControlElement + enum SlowControlElementType
// ---------------------------------------------------------------------

void init_slowcontrolelement(py::module_ &tf) {

    py::enum_<SlowControlElementType>(tf, "SlowControlElementType")
        .value("BUTTON", SlowControlElementType::BUTTON)
        .value("VARIABLE", SlowControlElementType::VARIABLE)
        .value("OPTIONS", SlowControlElementType::OPTIONS)
        .value("COMMAND", SlowControlElementType::COMMAND)
        .value("INFO", SlowControlElementType::INFO)
        .export_values();

    auto sce_cls = py::class_<SlowControlElement>(tf, "SlowControlElement")
        .def(py::init<std::string, SlowControlElementType, SCFunction, SCFunction, bool, bool>(),
             py::arg("name"), py::arg("type"),
             py::arg("change_function") = SCFunction(nullptr),
             py::arg("read_function") = SCFunction(nullptr),
             py::arg("lockable") = true, py::arg("hidden") = false)

        .def("GetName", &SlowControlElement::GetName)
        .def("IsName", &SlowControlElement::IsName, py::arg("name"))
        .def("Print", &SlowControlElement::Print)
        .def("JsonParser", &SlowControlElement::JsonParser, py::arg("json"))
        .def("GetChangeFunction", &SlowControlElement::GetChangeFunction)
        .def("GetReadFunction", &SlowControlElement::GetReadFunction)
        .def("GetType", &SlowControlElement::GetType)
        .def("AddCommand", &SlowControlElement::AddCommand, py::arg("value"))
        .def("SetDefault", &SlowControlElement::SetDefault, py::arg("value"))
        .def("Lockable", &SlowControlElement::Lockable)
        .def("Hidden", &SlowControlElement::Hidden)

        .def("SetValueString",
             static_cast<bool (SlowControlElement::*)(std::string)>(&SlowControlElement::SetValue),
             py::arg("value"))

        .def("GetValueStringChecked",
             [](SlowControlElement &self) {
                 std::string value;
                 bool ok = self.GetValue(value);
                 return py::make_tuple(ok, value);
             })
        ;

    sce_cls
        .def("SetValueInt",
             [](SlowControlElement &self, int value) { return self.SetValue<int>(value); })
        .def("SetValueDouble",
             [](SlowControlElement &self, double value) { return self.SetValue<double>(value); })

        .def("GetValueInt",
             [](SlowControlElement &self) { return self.GetValue<int>(); })
        .def("GetValueDouble",
             [](SlowControlElement &self) { return self.GetValue<double>(); })
        .def("GetValueString",
             [](SlowControlElement &self) { return self.GetValue<std::string>(); })

        .def("SetMinInt", [](SlowControlElement &self, int v) { return self.SetMin<int>(v); })
        .def("SetMinDouble", [](SlowControlElement &self, double v) { return self.SetMin<double>(v); })
        .def("SetMaxInt", [](SlowControlElement &self, int v) { return self.SetMax<int>(v); })
        .def("SetMaxDouble", [](SlowControlElement &self, double v) { return self.SetMax<double>(v); })
        .def("SetStepInt", [](SlowControlElement &self, int v) { return self.SetStep<int>(v); })
        .def("SetStepDouble", [](SlowControlElement &self, double v) { return self.SetStep<double>(v); })

        .def("AddOptionString", [](SlowControlElement &self, std::string v) { return self.AddOption<std::string>(v); })
        .def("AddOptionInt", [](SlowControlElement &self, int v) { return self.AddOption<int>(v); })
        .def("AddOptionDouble", [](SlowControlElement &self, double v) { return self.AddOption<double>(v); })
        ;
}

// ---------------------------------------------------------------------
// Binding class SlowControlCollection
// ---------------------------------------------------------------------

void init_slowcontrolcollection(py::module_ &tf) {
    py::class_<SlowControlCollection>(tf, "SlowControlCollection")

        .def("ListenForData", &SlowControlCollection::ListenForData,
             py::arg("poll_length") = 0)

        .def("Add", &SlowControlCollection::Add,
             py::arg("name"), py::arg("type"),
             py::arg("change_function") = SCFunction(nullptr),
             py::arg("read_function") = SCFunction(nullptr),
             py::arg("testing_lock") = true, py::arg("hidden") = false)

        .def("Remove", &SlowControlCollection::Remove, py::arg("name"))
        .def("Clear", &SlowControlCollection::Clear)

        .def("AlertSubscribe", &SlowControlCollection::AlertSubscribe,
             py::arg("alert"), py::arg("function"))
        .def("AlertSend", &SlowControlCollection::AlertSend,
             py::arg("alert"), py::arg("payload") = "")

        .def("Print", &SlowControlCollection::Print)
        .def("PrintJSON", &SlowControlCollection::PrintJSON)
        .def("Stop", &SlowControlCollection::Stop)
        .def("JsonParser", &SlowControlCollection::JsonParser, py::arg("json"))
        .def("TestingEnable", &SlowControlCollection::TestingEnable)
        .def("TestingDisable", &SlowControlCollection::TestingDisable)

        .def("__getitem__",
             [](SlowControlCollection &self, std::string key) -> SlowControlElement* {
                 return self[key];
             },
             py::return_value_policy::reference_internal)

        .def("GetValueDouble",
             [](SlowControlCollection &self, std::string name) { return self.GetValue<double>(name); })
        .def("GetValueInt",
             [](SlowControlCollection &self, std::string name) { return self.GetValue<int>(name); })
        .def("GetValueString",
             [](SlowControlCollection &self, std::string name) { return self.GetValue<std::string>(name); })
        ;
}

// ---------------------------------------------------------------------
// Enum LogLevel (Services.h)
// ---------------------------------------------------------------------

void init_loglevel(py::module_ &tf) {
    py::enum_<LogLevel>(tf, "LogLevel")
        .value("Error", LogLevel::Error)
        .value("Warning", LogLevel::Warning)
        .value("Message", LogLevel::Message)
        .value("Debug", LogLevel::Debug)
        .value("Debug1", LogLevel::Debug1)
        .value("Debug2", LogLevel::Debug2)
        .value("Debug3", LogLevel::Debug3)
        .export_values();
}

// ---------------------------------------------------------------------
// Binding class DAQInterface
// ---------------------------------------------------------------------

void init_daqinterface(py::module_ &tf) {
    py::class_<DAQInterface>(tf, "DAQInterface")
        .def(py::init<std::string>(), py::arg("configuration_file"))

        .def("SQLQueryList",
             static_cast<bool (DAQInterface::*)(const std::string&, std::vector<std::string>&, const unsigned int)>(&DAQInterface::SQLQuery),
             py::arg("query"), py::arg("responses"), py::arg("timeout") = default_timeout)
        .def("SQLQuerySingle",
             static_cast<bool (DAQInterface::*)(const std::string&, std::string&, const unsigned int)>(&DAQInterface::SQLQuery),
             py::arg("query"), py::arg("response"), py::arg("timeout") = default_timeout)
        .def("SQLQueryExec",
             static_cast<bool (DAQInterface::*)(const std::string&, const unsigned int)>(&DAQInterface::SQLQuery),
             py::arg("query"), py::arg("timeout") = default_timeout)

        .def("SendLog", &DAQInterface::SendLog,
             py::arg("message"), py::arg("severity") = LogLevel::Message,
             py::arg("device") = "", py::arg("timestamp") = 0)

        .def("SendAlarm", &DAQInterface::SendAlarm,
             py::arg("message"), py::arg("critical") = false,
             py::arg("device") = "", py::arg("timestamp") = 0,
             py::arg("timeout") = default_timeout)

        .def("SendMonitoringData", &DAQInterface::SendMonitoringData,
             py::arg("json_data"), py::arg("subject"), py::arg("device") = "", py::arg("timestamp") = 0)

        .def("SendCalibrationData",
             [](DAQInterface &self, const std::string& json_data, const std::string& description,
                const std::string& device, uint64_t timestamp, unsigned int timeout) {
                 int version = -1;
                 bool ok = self.SendCalibrationData(json_data, description, device, timestamp, &version, timeout);
                 return py::make_tuple(ok, version);
             },
             py::arg("json_data"), py::arg("description"), py::arg("device") = "",
             py::arg("timestamp") = 0, py::arg("timeout") = default_timeout)

        .def("GetCalibrationData",
             [](DAQInterface &self, const std::string& device, unsigned int timeout) {
                 std::string json_data;
                 int version = -1;
                 bool ok = self.GetCalibrationData(json_data, version, device, timeout);
                 return py::make_tuple(ok, json_data, version);
             },
             py::arg("device") = "", py::arg("timeout") = default_timeout)

        .def("SendDeviceConfig",
             [](DAQInterface &self, const std::string& json_data, const std::string& author,
                const std::string& description, const std::string& device, uint64_t timestamp, unsigned int timeout) {
                 int version = -1;
                 bool ok = self.SendDeviceConfig(json_data, author, description, device, timestamp, &version, timeout);
                 return py::make_tuple(ok, version);
             },
             py::arg("json_data"), py::arg("author"), py::arg("description"),
             py::arg("device") = "", py::arg("timestamp") = 0, py::arg("timeout") = default_timeout)

        .def("GetDeviceConfig",
             [](DAQInterface &self, int version, const std::string& device, unsigned int timeout) {
                 std::string json_data;
                 bool ok = self.GetDeviceConfig(json_data, version, device, timeout);
                 return py::make_tuple(ok, json_data);
             },
             py::arg("version"), py::arg("device") = "", py::arg("timeout") = default_timeout)

        .def("GetRunConfig",
             [](DAQInterface &self, int base_config_id, int runmode_config_id, unsigned int timeout) {
                 std::string json_data;
                 bool ok = self.GetRunConfig(json_data, base_config_id, runmode_config_id, timeout);
                 return py::make_tuple(ok, json_data);
             },
             py::arg("base_config_id"), py::arg("runmode_config_id"), py::arg("timeout") = default_timeout)

        .def("GetRunModeConfig",
             [](DAQInterface &self, const std::string& name, int version, unsigned int timeout) {
                 std::string json_data;
                 bool ok = self.GetRunModeConfig(json_data, name, version, timeout);
                 return py::make_tuple(ok, json_data);
             },
             py::arg("name"), py::arg("version"), py::arg("timeout") = default_timeout)

        .def("GetDeviceConfigFromRunConfig",
             [](DAQInterface &self, int base_config_id, int runmode_config_id, const std::string& device, unsigned int timeout) {
                 std::string json_data;
                 bool ok = self.GetDeviceConfigFromRunConfig(json_data, base_config_id, runmode_config_id, device, timeout);
                 return py::make_tuple(ok, json_data);
             },
             py::arg("base_config_id"), py::arg("runmode_config_id"), py::arg("device") = "", py::arg("timeout") = default_timeout)

        .def("SendROOTplot",
             [](DAQInterface &self, const std::string& plot_name, const std::string& draw_options,
                const std::string& json_data, uint64_t timestamp, unsigned int lifetime, unsigned int timeout) {
                 int version = -1;
                 bool ok = self.SendROOTplot(plot_name, draw_options, json_data, &version, timestamp, lifetime, timeout);
                 return py::make_tuple(ok, version);
             },
             py::arg("plot_name"), py::arg("draw_options"), py::arg("json_data"),
             py::arg("timestamp") = 0, py::arg("lifetime") = 5, py::arg("timeout") = default_timeout)

        .def("GetROOTplot",
             [](DAQInterface &self, const std::string& plot_name, unsigned int timeout) {
                 std::string draw_option, json_data;
                 int version = -1;
                 bool ok = self.GetROOTplot(plot_name, draw_option, json_data, version, timeout);
                 return py::make_tuple(ok, draw_option, json_data, version);
             },
             py::arg("plot_name"), py::arg("timeout") = default_timeout)

        .def("SendPlotlyPlot",
             [](DAQInterface &self, const std::string& name, const std::string& json_trace,
                const std::string& json_layout, uint64_t timestamp, unsigned int lifetime, unsigned int timeout) {
                 int version = -1;
                 bool ok = self.SendPlotlyPlot(name, json_trace, json_layout, &version, timestamp, lifetime, timeout);
                 return py::make_tuple(ok, version);
             },
             py::arg("name"), py::arg("json_trace"), py::arg("json_layout") = "{}",
             py::arg("timestamp") = 0, py::arg("lifetime") = 5, py::arg("timeout") = default_timeout)

        .def("GetPlotlyPlot",
             [](DAQInterface &self, const std::string& name, unsigned int timeout) {
                 std::string json_trace, json_layout;
                 int version = -1;
                 bool ok = self.GetPlotlyPlot(name, json_trace, json_layout, version, timeout);
                 return py::make_tuple(ok, json_trace, json_layout, version);
             },
             py::arg("name"), py::arg("timeout") = default_timeout)

        .def("GetLocalConfig", &DAQInterface::GetLocalConfig)
        .def("SetLocalConfig", &DAQInterface::SetLocalConfig, py::arg("json"))

        .def("GetSlowControlCollection", &DAQInterface::GetSlowControlCollection,
             py::return_value_policy::reference_internal)

        .def("GetSlowControlVariable", &DAQInterface::GetSlowControlVariable,
             py::arg("key"), py::return_value_policy::reference_internal)

        .def("AddSlowControlVariable", &DAQInterface::AddSlowControlVariable,
             py::arg("name"), py::arg("type"),
             py::arg("change_function") = std::function<std::string(const char*)>(nullptr),
             py::arg("read_function") = std::function<std::string(const char*)>(nullptr))

        .def("RemoveSlowControlVariable", &DAQInterface::RemoveSlowControlVariable, py::arg("name"))
        .def("ClearSlowControlVariables", &DAQInterface::ClearSlowControlVariables)
        .def("PrintSlowControlVariables", &DAQInterface::PrintSlowControlVariables)

        .def_property_readonly("sc_vars",
             [](DAQInterface &self) -> SlowControlCollection& { return self.sc_vars; },
             py::return_value_policy::reference_internal)

        .def("AlertSubscribe", &DAQInterface::AlertSubscribe, py::arg("alert"), py::arg("function"))
        .def("AlertSend", &DAQInterface::AlertSend, py::arg("alert"), py::arg("payload"))

        .def("GetDeviceName", &DAQInterface::GetDeviceName)
        .def("SetVerbose", &DAQInterface::SetVerbose, py::arg("in"))

        .def("GetSlowControlValueDouble",
             [](DAQInterface &self, std::string name) { return self.GetSlowControlValue<double>(name); })
        .def("GetSlowControlValueInt",
             [](DAQInterface &self, std::string name) { return self.GetSlowControlValue<int>(name); })
        .def("GetSlowControlValueString",
             [](DAQInterface &self, std::string name) { return self.GetSlowControlValue<std::string>(name); })
        ;
}

PYBIND11_MODULE(daqinterface, m) {
    m.doc() = "Binding Python for ToolDAQ / ToolFramework (libToolDAQ)";

    py::module_ tf = m.def_submodule("ToolFramework",
        "Classes of namespace ToolFramework (libToolDAQ)");

    init_store(tf);
    init_slowcontrolelement(tf);
    init_slowcontrolcollection(tf);
    init_loglevel(tf);
    init_daqinterface(tf);
}
