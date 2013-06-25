# Hosts

Each node in the home automation network is a host. This directory stores the source code for all kinds of hosts.

* __switch__: source code for hosts that act as power switchs. Generaly used by wall switchs to activate lamps.
* __relay__: source code for hosts that just relay packets to other hosts.
* __router__: source code for the router host, responsible to provide the interface between the radio network and external networks (like ethernet). This host can route command packets from outside the radio network allowing a external applications to operate the home devices.

