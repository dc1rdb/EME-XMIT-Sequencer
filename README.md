# EME-XMIT-Sequencer
Quick and dirty sequencer for my EME station using PTT signal from WSJT-X

This is a little sequencer to properly switch LNA, PA and XCVR of my EME station.

It works with an Arduino Uno clone and Arduino relay shield. The PTT signal comes directly from WSJT-X via USB (RTS). Therefore, an additional wire between CH340G pin 13 and GPIO 2 on Arduino Uno clone board is required.

For TXing outside WSJT-X, an external PTT input is provided.

Optionally, the sequencer provides properly sequenced ANT pol change between RX and TX cycles to cope with some Faraday rotation effects.

Please note that the XCVR should be exclusively controlled by the sequencer to avoid burning relays and LNAs ;-)

![IMG_20251228_080318~2](https://github.com/user-attachments/assets/d18ade7c-3f1a-4d0a-808a-59478e7adf4f)
![IMG_20251228_080343~2](https://github.com/user-attachments/assets/f59f0ab4-40f8-4584-be56-8898ae08559f)
![IMG_20251228_080509~2](https://github.com/user-attachments/assets/4fdec8d0-1dd9-481b-b690-df3e18f93065)
