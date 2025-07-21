# Explain E2Sim's Functionalities
This is an update to E2 Simulator, based on E2AP v1 defined in ORAN
E2 Simulator is built as a library that supports basic functions and is
linked by an E2 Simulation application.

The E2 Simulator breaks up the simulation roles into two:
1. Base E2AP support
2. E2SM Support

#1 is the role supported by E2 Simulator library, while #2 is supported by the
calling application.  An example of a calling application is provided in this repo
under the

E2 Simulator enables a specific E2SM to be supported by the calling application in
the following way:

* E2SM must be registered by the calling application
* E2SM-specific code uses callbacks to handle interaction from xApp
* E2SM-specific code must continuously implement simulation logic

Registration of an E2SM with the E2 Simulator entails the following:
* RAN function definition
* Callback functions for any of the following:
  Subscription
  Control
  Any Responses

The following is the E2 Simulator Main Program Flow
1. Upon startup, Simulator generates E2 Setup Request
   For each E2SM that is registered, include RAN function definition

2. Upon receiving incoming requests, decode
   Identify RAN function ID
   Call appropriate callback function for RAN function ID and request type
   Provide entire encoded request
   If no callback is provided, we return an error response
3. Upon receiving responses, decode:
   Identify RAN function ID
   If a callback is registered for this message type, call it based on RAN function ID and request type
   Provide entire encoded response
   If no callback is registered, no action is taken

E2SM Callback Functions:

* Callback functions are responsible for sending responses
* They may also need to set up ongoing simulation messaging
  REPORT Service sends back periodic Indication messages containing REPORT
  INSERT Service sends back periodic Indication messages containing INSERT
* They may need to associate incoming messages with existing service invocation
  Modify Subscription
  CONTROL message associated with INSERT
* Base E2 simulator should not be concerned with these details; only specific E2SM code should be responsible for any messaging

In K8s environment, multiple instances of E2Sim can run in parallel to simulate an environment with multiple cells/UEs. Each instance will have a random RAN function ID and random `gnb` name.

To set the ID of RAN function, use environment variable `RAN_FUNC_ID`. If it is not set, the ID will be assigned to a random value between 0 and 4095.
