# Hosts

Each node in the home automation network is a host. This directory stores the source code for all kinds of hosts.

* __switch__ Source code for hosts that act as power switchs. Generaly used by wall switch to activate lamps.
* __relay__ Source code for hosts that just relay packets to other hosts.
* __router__ Source code for the router host, responsible to make an interface between the radio network and external networks (like ethernet ones). This host can route instructions from outside the radio network, allowing a mobile app (or something else) operate the home devices.

