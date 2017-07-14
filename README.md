# EasyDLP
EasyDLP is a smart and simple data loss prevention system. It has a simple user interface which provides full control of endpoint computers.

# Components
The system consists of 3 main components
  - Server
    - Stores and handles incidents, rules and running endpoints
  - Administration suite
    - Provides an easy to use graphical interface to monitor and manage rules, incidents and endpoints
  - Endpoint client
    - Intercepts IO requests, notifies incidents and **prevents the actual data leak** 

# In-depth
The main idea behind EasyDLP is IO requests interception.
IO requests like opening a file is the most important request which can be intercepted by the system.

#### General idea
The endpoint client intercepts all of the calls to `CreateFileW` from a chosen process.
After the call is intercepted the client validates the file against a set of rules set by the administrator. 
If the file complies to the given set of rules it will be forwarded to the process.
If the file doesn't fit the rules, an incident will be reported and the file will not be accessible.

#### The interception process
There are multiple ways for IO requests interception. In this system the method which was used to intercept is *DLL injection* and *API hooking*.

The setup of the interception is as follows:
-   Locate a dangerous process
-   Inject a DLL to this process 
    -   The injection is done using the `CreateRemoteThread` and `LoadLibrary` method
-   Hook `CreateFileW` using the *Mhook* library
-   Fetch rules from the server

This is a simple diagram which demonstrates the interception process
![Mhook in action](http://codefromthe70s.org/images/mhook_diagram.png)
