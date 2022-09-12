== DSMR component for ESPHome

Block diagram of the code:

```
   +-------------+
   | Smart Meter |
   | (physical)  |
   +-------------+
          | Serial P1 port                       .------> logging
          |                                     /  .----> sensors
          v                                    /  /  .--> ...
   +----------------+       +----------------+ --'  /
   | UART component |       | Event Handlers | ----'
   +----------------+       +----------------+
          |                          ^
          | bytes                    | events
          v                          |
   +-----------+            +----------------+
   | DsmrInput |---bytes--->| TelegramParser |
   +-----------+            +----------------+
```

Responsibilities:

* **Smart Meter**: Provides smart meter metrics through DSMR telegrams on the P1 port.
* **UART component**: Reads and buffers received P1 port data.
* **DsmrInput**: Transports received port data from the UART component to the DsmrReader.
  The main reason for having this funtionality separated, is that it makes it possible to
  implement other inputs that get their data from arbitrary sources (e.g. to write a
  DsmrInput that provides static data for unit testing purposes).
* **DsmrReader**: Transports received port data to the TelegramParser, while keeping an
  eye out for read timeouts and keeping the data stream going for cases where the DsmrInput
  might not be able to fully buffer the incoming data stream.
* **TelegramParser**: Interprets the incoming data stream (e.g. finding the start of a
  DSMR telegram, decrypting encrypted telegrams, translating the data into metrics) and
  fires various events for interested Event Handlers.
* **Event Handlers**: Listen to events from the TelegramParser, for example to publish
  new metrics using sensor components.