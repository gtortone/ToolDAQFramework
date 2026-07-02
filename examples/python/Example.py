import time # for sleep
import random # for random
import ctypes
import daqinterface as di

class AutomatedFunctions:
  DAQ_inter = 0
  name=0
  
  def __init__(self, iDAQ_inter, iname):
    self.DAQ_inter = iDAQ_inter
    self.name = iname
  
  def new_event_func(self, event_name: str, event_payload: str) -> bool:
    print("new_event_func fired for event ",event_name)
    self.DAQ_inter.SendLog(self.name+" received an alert for event "+event_name)
    # add your desired actions on new_event here
    # if this function is subscribed to multiple alerts, you can use event_name
    # to determine the appropriate actions to take
    # return type for AlertSubscribe functions is void (no return value)
    return true

  def start_func(self, control_name: str) -> str:
    self.DAQ_inter.SendLog(self.name," received start signal")
    # add code to perform any startup actions here
    ret = "Service Started"
    return ret
  
  def change_voltage(self, control_name: str) -> str:
    print("voltage change callback for control ",control_name)
    # the same function can be registered as a callback for multiple controls;
    # the changed control name is received as an argument.
    # so e.g. this function may be registered to controls for multiple HV channels
    # (each with a unique name), with the name received identifying the channel altered.
    # get the new value of the control (i.e. the requested voltage)
    new_voltage = self.DAQ_inter.sc_vars[control_name].GetValueDouble()
    # add appropriate code to enact changing the voltage here
    msg = self.name + " setting voltage of "+control_name+" to "+str(new_voltage)
    self.DAQ_inter.SendLog(msg)
    # the function should return a string, for example indicating success/failure
    ret = control_name+" set to "+str(new_voltage)+"V"
    return ret
  
if __name__ == "__main__":
  
  # configuration
  Interface_configfile = "./InterfaceConfig"
  #database_name = "daq"
  
  print("Initialising daqinterface")
  # initialise DAQInterface
  DAQ_inter = di.ToolFramework.DAQInterface(Interface_configfile)
  device_name = DAQ_inter.GetDeviceName()
  automated_functions = AutomatedFunctions(DAQ_inter, device_name)
  
  # set initial status so we can track when the service is ready
  DAQ_inter.sc_vars["Status"].SetValueString("Initialising")
  
  ###############################################################
  ####### demonstrate sending logging messages and alarms #######
  ###############################################################
  
  print("Testing logging")
  # send a log message to the database, specifing severity and device name
  # severity 0 is critical, higher numbers => lower severity
  DAQ_inter.SendLog("important message", di.ToolFramework.Error, device_name)
  DAQ_inter.SendLog("severity 2 message", di.ToolFramework.Message, device_name)
  # we may omit the severity and/or logging source, in which case a default severity of 9 will be used,
  # and the logging source will be the device name we passed to the DAQInterface constructor
  DAQ_inter.SendLog("unimportant message")
  # the signature for sending alarms is the same as logging messages
  DAQ_inter.SendAlarm("High current on channel 3", True, device_name)
  
  ###############################################################
  #######        registering subscriber functions         #######
  ###############################################################
  
  print("Testing AlertSubscribe")
  # we can register functions to be invoked in response to a broadcast event,
  # such as the start of a new run or when the program state changes (an example is given later).
  # we register a function with the DAQInterface::AlertSubscribe method.
  # for python we must pass a variable assigned to the desired function,
  # and that variable must still be in scope at any time it may be invoked.
  # i.e. we cannot pass 'automated_functions.new_event_func' directly as per:
  #DAQ_inter.AlertSubscribe("new_event", automated_functions.new_event_func)
  # but instead we must do this:
  new_event_func_ref = automated_functions.new_event_func
  DAQ_inter.AlertSubscribe("new_event", new_event_func_ref)
  
  ###############################################################
  #######           registering slow controls             #######
  ###############################################################
  
  print("Adding controls")
  # We can register controls associated with our service via the DAQ_inter.sc_vars.Add method
  # this takes a control name and type, as a minimum.
  # available types are: { BUTTON, VARIABLE, OPTIONS, COMMAND, INFO }
  DAQ_inter.sc_vars.Add("Info",di.ToolFramework.INFO)
  
  # Use DAQ_inter.sc_vars["ControlName"] to access a SlowControlElement.
  # Use the SetValue method of a control to update its value displayed on the webpage
  DAQ_inter.sc_vars["Info"].SetValueString(" hello this is an information message ,.!{}[]<>?/`~'@\" ")
  
  # Typical controls might include buttons for starting, stopping and quitting a service
  DAQ_inter.sc_vars.Add("Start",di.ToolFramework.BUTTON)
  DAQ_inter.sc_vars["Start"].SetValueInt(False)
  
  DAQ_inter.sc_vars.Add("Stop",di.ToolFramework.BUTTON)
  DAQ_inter.sc_vars["Stop"].SetValueInt(False)
  
  DAQ_inter.sc_vars.Add("Quit",di.ToolFramework.BUTTON)
  DAQ_inter.sc_vars["Quit"].SetValueInt(False)
  
  # We can make a radio button control with the OPTIONS control type
  DAQ_inter.sc_vars.Add("power_on",di.ToolFramework.OPTIONS)
  DAQ_inter.sc_vars["power_on"].AddOptionString("1")
  DAQ_inter.sc_vars["power_on"].AddOptionString("0")
  DAQ_inter.sc_vars["power_on"].SetValueString("0")
  
  # we can add variable controls with the VARIABLE control type
  # this additionally takes a range and step size
  DAQ_inter.sc_vars.Add("voltage_1", di.ToolFramework.VARIABLE)
  DAQ_inter.sc_vars["voltage_1"].SetMinInt(0)
  DAQ_inter.sc_vars["voltage_1"].SetMaxInt(5000)
  DAQ_inter.sc_vars["voltage_1"].SetStepDouble(0.1)
  DAQ_inter.sc_vars["voltage_1"].SetValueDouble(3500.5)
  
  ###############################################################
  #######           querying control values               #######
  ###############################################################
  
  print("Querying controls")
  voltage_1 = DAQ_inter.sc_vars["voltage_1"].GetValueDouble()
  
  ###############################################################
  #######           registering callbacks                 #######
  ###############################################################
  
  print("Registering control callbacks")
  # similar to subscriber functions, we can register a callback function to be invoked
  # whenever a control value is changed. This enables us to act on changes when they happen
  # without the need to constantly poll the DAQInterface to identify when something changes.
  change_voltage_ref = automated_functions.change_voltage
  DAQ_inter.sc_vars.Add("voltage_2", di.ToolFramework.VARIABLE, change_voltage_ref)
  DAQ_inter.sc_vars["voltage_2"].SetMinInt(0)
  DAQ_inter.sc_vars["voltage_2"].SetMaxInt(5000)
  DAQ_inter.sc_vars["voltage_2"].SetStepInt(10)
  DAQ_inter.sc_vars["voltage_2"].SetValueInt(4000)
  
  ###############################################################
  #######  retrieving configurations from the database    #######
  ###############################################################
  
  # configuration entries are uniquely identified by a pair of {device name, version number}
  # and are represented in the database as JSON strings. To query for this device's configuration:
  print("Querying DB")
  version=-1  # use version -1 to get the latest version
  config_json = ""
  ok = DAQ_inter.GetDeviceConfig(version, config_json)
  
  # a Store class instance can parse a JSON string to more easily access settings
  # and can generate a JSON string from contents set by a series of simple 'Set' calls
  # so is useful both for reading and writing configuration entries
  configuration = di.ToolFramework.Store()
  
  # local variables to retain current values
  power_on = ctypes.c_bool()
  voltage_1 = ctypes.c_float()
  voltage_2 = ctypes.c_float()
  
  # check if a configuration was found in the database
  if not ok or config_json=="":
    
    print("Making new DB entry")
    # no matching configuration entry was found. let's make one.
    power_on.value = False  
    configuration.SetBool("power_on", power_on)
    configuration.SetDouble("voltage_1", DAQ_inter.sc_vars["voltage_1"].GetValueDouble())
    configuration.SetDouble("voltage_2", 4000.)
    
    # upload configuration to the database
    print("sending new config_json: '",str(configuration),"'")
    #DAQ_inter.SendDeviceConfig(config_json, "DemoAuthor", "Demo Description")
    
  else:
    
    print("Parsing query response")
    # we got a configuration JSON string. Parse it with the Store class
    print("got config JSON: '",config_json,"'")
    configuration.JsonParser(config_json)
    
    # print the configuration
    configuration.Print()
    
    # retrieve the configuration variables and use them to set initial control values
    configuration.GetBool("power_on", power_on)
    DAQ_inter.sc_vars["power_on"].SetValueInt(power_on)
    DAQ_inter.sc_vars["voltage_1"].SetValueDouble(configuration.GetDouble("voltage_1"))
    
    # if the Store does not contain a given key
    # (i.e. no corresponding setting was in the database configuration entry)
    # the Store::Get method will return False
    if configuration.GetDouble("voltage_2", voltage_2) == True:
      DAQ_inter.sc_vars["voltage_2"].SetValueDouble(voltage_2)
    else:
      # use a default value if no setting is available
      voltage_2 = 2000
      # and report this to the logging database
      DAQ_inter.SendLog("voltage_2 not set in configuration version "+str(version), di.ToolFramework.Error, device_name)
    

  ###############################################################
  #######           querying the database                 #######
  ###############################################################
  
  print("testing generic sql queries")
  # single-record query
  resp = ""
  qryok = DAQ_inter.SQLQuerySingle("SELECT time, message FROM logging ORDER BY time DESC LIMIT 1",resp)
  print("single-record query success: ",qryok,", response: '",resp,"'")
  
  # for multi-record queries
  resps = []
  qryok = DAQ_inter.SQLQueryList("SELECT time, message FROM logging ORDER BY time DESC LIMIT 5",resps)
  print("multi-record query success: ",qryok,", responses:")
  for i in range(min(5,len(resps))):
     print(i,": '",resps[i],"'")
  if len(resps) > 5:
     print("...\n")
  
  ###############################################################
  #######                  Main Program Loop              #######
  ###############################################################
  
  print("main loop")
  # We'll use a Store to accumulate monitoring data
  # and convert to json to send to the webpage for plotting
  monitoring_data = di.ToolFramework.Store()
  
  running = True
  last_started = True;
  started = False;
  
  DAQ_inter.sc_vars["Status"].SetValueString("Ready")
  
  while running:
    
    if last_started==True and started==False:
      print("Waiting for user to click 'Start' or 'Quit'...")
    
    # update quit check
    running=(not DAQ_inter.sc_vars["Quit"].GetValueInt())
    
    # check for Start control being clicked
    #######################################
    started = DAQ_inter.sc_vars["Start"].GetValueInt()
    if started:
      print("Got Start signal")
      # (n.b. we could have alternatively defined these actions in automated_functions.start_func)
      # update the status control to indicate our new program state
      DAQ_inter.sc_vars["Status"].SetValueString("Running")
      
      # fire all functions subscribed to the 'new_event' signal
      print("sending 'new_event' alert")
      DAQ_inter.sc_vars.AlertSend("new_event")
      
      # reset the 'Start' button, so that we can accept new presses
      DAQ_inter.sc_vars["Start"].SetValueInt(False)
      
    last_started = started;
    
    while started:
      
      # check for Quit control being clicked
      #######################################
      running = (not DAQ_inter.sc_vars["Quit"].GetValueInt())
      
      # if clicked, update our status indicator and reset the control buttons
      if DAQ_inter.sc_vars["Stop"].GetValueInt() or not running:
        print("Got stop signal")
        started = False
        DAQ_inter.sc_vars["Status"].SetValueString("Stopped")
        DAQ_inter.sc_vars["Stop"].SetValueInt(False)
        DAQ_inter.sc_vars["Start"].SetValueInt(False)
      
      # Report current values to monitoring plots
      ###########################################
      
      # clear any values from the monitoring Store
      monitoring_data.Delete()
      # Note that 'Store::Set' calls to existing keys will overwrite old values, so calling 'Delete'
      # just ensures no entries that weren't 'Set' before it's invoked carry over.
      # Of course in the specific case here we make the same series of 'Set' calls on every loop,
      # so calling 'Delete' is redundant as all keys will either be overwritten or re-created.
      
      # populate the monitoring Store
      print("setting monitoring vals")
      monitoring_data.SetDouble("temp_1", 30+random.random())
      monitoring_data.SetDouble("temp_2", 28+random.random())
      monitoring_data.SetDouble("temp_3", 18+random.random())
      monitoring_data.SetDouble("current_1", random.uniform(0,5))
      monitoring_data.SetDouble("current_2", random.uniform(0,5))
      monitoring_data.SetDouble("voltage_1", DAQ_inter.sc_vars["voltage_1"].GetValueDouble())
      monitoring_data.SetDouble("voltage_2", DAQ_inter.sc_vars["voltage_2"].GetValueDouble())
      monitoring_data.SetInt("power_on", DAQ_inter.sc_vars["power_on"].GetValueInt())
      
      # generate a JSON from the contents
      monitoring_json = str(monitoring_data) 
      
      # send to the Database for plotting on the webpage
      DAQ_inter.SendMonitoringData(monitoring_json, "general")
      
      # retrieve and respond to control changes
      ###########################################
      print("checking for updated controls")
      power_on = DAQ_inter.sc_vars["power_on"].GetValueInt()
      if not power_on:
        if DAQ_inter.sc_vars["voltage_1"].GetValueInt():
          DAQ_inter.sc_vars["voltage_1"].SetValueDouble(0.0)
        if DAQ_inter.sc_vars["voltage_2"].GetValueInt():
          DAQ_inter.sc_vars["voltage_2"].SetValueDouble(0.0)
      
      # voltage_2 is changed automatically via automated_functions.voltage_change
      # but here is an example for voltage_1 of doing it manually:
      # get the current control value from DAQ_inter, and compare with last known setting
      if DAQ_inter.sc_vars["voltage_1"].GetValueDouble() != voltage_1:
        
        # if the control value has changed, update the local variable holding last known value
        voltage_1 = DAQ_inter.sc_vars["voltage_1"].GetValueDouble()
        print("voltage_1 change requested; new value is ",voltage_1)
        
      # limit loop rate so we don't lock up the CPU
      #############################################
      #print("inner sleep")
      time.sleep(1)
    
    # end of operation loop
    
    #print("outer sleep")
    time.sleep(1)
    
  # end of program loop
  print("end main loop")
  
  DAQ_inter.sc_vars["Status"].SetValueString("Terminated")
