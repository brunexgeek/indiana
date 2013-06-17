# Indiana nRF24L01+ Radio Library

This is a library for using the nrf24L01+ modules and include an optional network layer to deal with packet routing and logical addressing (this means you can create a tree or mesh network). This library was developed to be portable and simple.

With this implementation you will be able to use the auto acknowledgement and auto retransmission features of the nrf24L01+ (only static length payload for now). The network layer allow the both features, but reimplement them via software because we need some additional control over the comunication.

This code was inspired by two other implementations: "nrf24L01+ Radio Library" (https://github.com/kehribar/nrf24L01_plus) by kehribar and "Arduino Driver for nRF24L01" (https://github.com/maniacbug/RF24/) by maniacbug.

## Configuration

When you call the initialization function, then device will be configured with some default values. 

    nrf24_initialize();

No pipes will be enabled for use yet. To enable a pipe for receive packets, use the "openReadingPipe" function.

    // enable the pipe 0 for receive through the address 0x33cc3301
    nrf24_openReadingPipe(0, 0x33cc3301);

The nRF24L01+ library offer a set of functions to configure some parameters, like the channel and payload length.

    nrf24_setChannel(1);

(under construction)
