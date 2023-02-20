//@main.c
#include "N76E003.h"

#include "delay.h"
#include "radio.h"

uint8_t _channel = DEFAULT_CHANNEL;

uint8_t SPITransfer(uint8_t x) {
  SPDR = x;
  Timer3_Delay10us(1);
  while (!(SPSR & 0x80))
    ;
  clr_SPIF;
  return SPDR;
}

uint16_t LT8920ReadRegister(uint8_t reg) {
  uint8_t h, l;

  SS = 0;
  Timer3_Delay10us(1);

  SPITransfer(REGISTER_READ | reg);
  h = SPITransfer(0);
  l = SPITransfer(0);

  SS = 1;
  return (h << 8) | l;
}

uint8_t LT8920WriteRegister2(uint8_t reg, uint8_t high, uint8_t low) {
  uint8_t result;

  SS = 0;
  Timer3_Delay10us(1);
  result = SPITransfer(REGISTER_WRITE | reg);
  SPITransfer(high);
  SPITransfer(low);

  SS = 1;
  return result;
}

uint8_t LT8920WriteRegister(uint8_t reg, uint16_t val) {
  return LT8920WriteRegister2(reg, val >> 8, val);
}

void LT8920Begin() {
  LT8920WriteRegister(0, 0x6fe0);
  LT8920WriteRegister(1, 0x5681);
  LT8920WriteRegister(2, 0x6617);
  LT8920WriteRegister(4, 0x9cc9); // why does this differ from powerup (5447)
  LT8920WriteRegister(5, 0x6637); // why does this differ from powerup (f000)
  LT8920WriteRegister(8, 0x6c90); // power (default 71af) UNDOCUMENTED

  LT8920SetCurrentControl(4, 0); // power & gain.

  LT8920WriteRegister(10, 0x7ffd); // bit 0: XTAL OSC enable
  LT8920WriteRegister(
      11, 0x0000); // bit 8: Power down RSSI (0=  RSSI operates normal)
  LT8920WriteRegister(12, 0x0000);
  LT8920WriteRegister(13, 0x48bd); //(default 4855)

  LT8920WriteRegister(22, 0x00ff);
  LT8920WriteRegister(23,
                      0x8005); // bit 2: Calibrate VCO before each Rx/Tx enable
  LT8920WriteRegister(24, 0x0067);
  LT8920WriteRegister(25, 0x1659);
  LT8920WriteRegister(26, 0x19e0);
  LT8920WriteRegister(27, 0x1300); // bits 5:0, Crystal Frequency adjust
  LT8920WriteRegister(28, 0x1800);

  // fedcba9876543210
  LT8920WriteRegister(
      32, 0x5000); // AAABBCCCDDEEFFFG  A preamble length, B, syncword length, c
                   // trailer length, d packet type
  //                  E FEC_type, F BRCLK_SEL, G reserved
  // 0x5000 = 0101 0000 0000 0000 = preamble 010 (3 bytes), B 10 (48 bits)
  LT8920WriteRegister(33, 0x3fc7);
  LT8920WriteRegister(34, 0x2000); //
  LT8920WriteRegister(
      35, 0x0300); // POWER mode,  bit 8/9 on = retransmit = 3x (default)
  LT8920SetSyncWord(0x03805a5a, 0x03800380);

  LT8920WriteRegister(40,
                      0x4401); // max allowed error bits = 0 (01 = 0 error bits)
  LT8920WriteRegister(R_PACKETCONFIG, PACKETCONFIG_CRC_ON |
                                          PACKETCONFIG_PACK_LEN_ENABLE |
                                          PACKETCONFIG_FW_TERM_TX);

  LT8920WriteRegister(42, 0xfdb0);
  LT8920WriteRegister(43, 0x000f);

  // setDataRate(LT8920_1MBPS);

  LT8920WriteRegister(R_FIFO, 0x0000); // TXRX_FIFO_REG (FIFO queue)

  LT8920WriteRegister(R_FIFO_CONTROL, 0x8080); // Fifo Rx/Tx queue reset

  Timer2_Delay500us(400);
  LT8920WriteRegister(R_CHANNEL,
                      1 << CHANNEL_TX_BIT); // set TX mode.  (TX = bit 8, RX =
                                            // bit 7, so RX would be 0x0080)
  Timer2_Delay500us(4);
  LT8920WriteRegister(R_CHANNEL, _channel); // Frequency = 2402 + channel
}

void LT8920SetCurrentControl(uint8_t power, uint8_t gain) {
  LT8920WriteRegister(R_CURRENT,
                      ((power << CURRENT_POWER_SHIFT) & CURRENT_POWER_MASK) |
                          ((gain << CURRENT_GAIN_SHIFT) & CURRENT_GAIN_MASK));
}

void LT8920StartListening() {
  LT8920WriteRegister(R_CHANNEL, _channel & CHANNEL_MASK); // turn off rx/tx
  Timer2_Delay500us(6);
  LT8920WriteRegister(R_FIFO_CONTROL, 0x0080); // flush rx
  LT8920WriteRegister(R_CHANNEL, (_channel & CHANNEL_MASK) |
                                     (1 << CHANNEL_RX_BIT)); // enable RX
  Timer2_Delay500us(10);
}

int LT8920Read(uint8_t *buffer, size_t maxBuffer) {
  uint16_t value = LT8920ReadRegister(R_STATUS);
  uint8_t pos = 0;
  if ((value | (1 << STATUS_CRC_BIT)) == 0) {
    // CRC ok

    uint16_t val = LT8920ReadRegister(R_FIFO);
    uint8_t packetSize = val >> 8;
    if (maxBuffer < packetSize + 1) {
      // BUFFER TOO SMALL
      return -2;
    }

    buffer[pos++] = (val & 0xFF);
    while (pos < packetSize) {
      val = LT8920ReadRegister(R_FIFO);
      buffer[pos++] = val >> 8;
      buffer[pos++] = val & 0xFF;
    }

    return packetSize;
  } else
    // CRC error
    return -1;
}

void LT8920SetSyncWord(uint32_t syncWordLow, uint32_t syncWordHigh) {
  LT8920WriteRegister(R_SYNCWORD1, syncWordLow);
  LT8920WriteRegister(R_SYNCWORD2, syncWordLow >> 16);
  LT8920WriteRegister(R_SYNCWORD3, syncWordHigh);
  LT8920WriteRegister(R_SYNCWORD4, syncWordHigh >> 16);
}

void LT8920SetSyncWordLength(uint8_t option) {
  option &= 0x03;

  LT8920WriteRegister(32, (LT8920ReadRegister(32) & 0x0300) | (option << 11));
}

bool LT8920SendPacket(uint8_t *val, size_t packetSize) {
  uint8_t pos;
  if (packetSize < 1 || packetSize > 255)
    return false;

  // LT8920WriteRegister(R_CHANNEL, 0x0000);
  LT8920WriteRegister(R_FIFO_CONTROL, 0); // 0x8000);  //flush tx

  ////////////////////////////////////////////////////////
  LT8920WriteRegister(R_CHANNEL, (_channel & CHANNEL_MASK) |
                                     (1 << CHANNEL_TX_BIT)); // enable TX
  ////////////////////////////////////////////////////////

  // packets are sent in 16bit words, and the first word will be the packet
  // size. start spitting out words until we are done.

  pos = 0;
  LT8920WriteRegister2(R_FIFO, packetSize, val[pos++]);
  while (pos < packetSize) {
    uint8_t msb = val[pos++];
    uint8_t lsb = val[pos++];

    LT8920WriteRegister2(R_FIFO, msb, lsb);
  }

  // LT8920WriteRegister(R_CHANNEL,  (_channel & CHANNEL_MASK) |
  // (1<<CHANNEL_TX_BIT));   //enable TX

  // Wait until the packet is sent.
  /*while (digitalRead(_pin_pktflag) == 0)
  {
      //do nothing.
  }*/

  return true;
}

void LT8920SetChannel(uint8_t channel) {
  _channel = channel;
  LT8920WriteRegister(R_CHANNEL, (_channel & CHANNEL_MASK));
}

bool LT8920Available() { return (LT8920ReadRegister(48) & (1 << 6)) != 0; }

#define _BV(n) (1 << (n))

void LT8920ScanRSSI(uint16_t *buffer, uint8_t start_channel,
                    uint8_t num_channels) {
  // LT8920WriteRegister(R_CHANNEL, _BV(CHANNEL_RX_BIT));
  //
  // //add read mode.
  LT8920WriteRegister(R_FIFO_CONTROL, 0x8080); // flush rx
  // LT8920writeRegister(R_CHANNEL, 0x0000);

  // set number of channels to scan.
  LT8920WriteRegister(42, (LT8920ReadRegister(42) & 0b0000001111111111) |
                              ((num_channels - 1 & 0b111111) << 10));

  // set channel scan offset.
  LT8920WriteRegister(43, (LT8920ReadRegister(43) & 0b0000000011111111) |
                              ((start_channel & 0b1111111) << 8));
  LT8920WriteRegister(43,
                      (LT8920ReadRegister(43) & 0b0111111111111111) | _BV(15));

  while (!LT8920Available())
    ;

  // read the results.
  uint8_t pos = 0;
  while (pos < num_channels) {
    uint16_t data = LT8920ReadRegister(R_FIFO);
    buffer[pos++] = data >> 8;
  }
}

void LT8920BeginScanRSSI(uint8_t start_channel, uint8_t num_channels) {
  // LT8920WriteRegister(R_CHANNEL, _BV(CHANNEL_RX_BIT));
  //
  // //add read mode.
  LT8920WriteRegister(R_FIFO_CONTROL, 0x8080); // flush rx
  // LT8920writeRegister(R_CHANNEL, 0x0000);

  // set number of channels to scan.
  LT8920WriteRegister(42, (LT8920ReadRegister(42) & 0b0000001111111111) |
                              ((num_channels - 1 & 0b111111) << 10));

  // set channel scan offset.
  LT8920WriteRegister(43, (LT8920ReadRegister(43) & 0b0000000011111111) |
                              ((start_channel & 0b1111111) << 8));
  LT8920WriteRegister(43,
                      (LT8920ReadRegister(43) & 0b0111111111111111) | _BV(15));
}

void LT8920EndScanRSSI(uint16_t *buffer, uint8_t num_channels) {
  while (!LT8920Available())
    ;

  // read the results.
  uint8_t pos = 0;
  while (pos < num_channels) {
    uint16_t data = LT8920ReadRegister(R_FIFO);
    buffer[pos++] = data >> 8;
  }
}