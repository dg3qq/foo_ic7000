/**
 * 
 * @breef Implement Icom CI-V protocol
 * 
 * @author DG3QQ
 *
 * The goal of this is to act as a Icom CI-V protocol from the
 * radio CAT point of view.
 * 
 * This work was a need from my side for testing device drivers of CHIRP
 * project.
 *
 * This is a experimantal/preliminary implementation of the protocol.
 *radio 
 * There meight be issues and deficiencies; not bomb proof yet.  
 * 
 * This code has been built with the review of various sources:
 * - Icom IC7000 manual
 * - df4or.de http://www.plicht.de/ekki/civ/civ-p3.html
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * **************************************************************************/

#pragma once

#define ICOM_DEFAULT_CIV_BAUDRATE 19200
//#define CIV_ADDR_RADIO 0x70 // depends on radio model
#define ICOM_DEFAULT_CIV_ADDR_CONTROL 0xE0 // proposed default 

#define CIV_ADDR_BROADCAST 0x00 // fixed by protocol specification

#define CIV_BUFFER_SIZE 64 // ?? check the docs


#define CIV_B_START 0xFE
#define CIV_B_END 0xFD
// OK MESSAGE TO CONTROLLER // FE FE 70 E0 FB FD
#define CIV_B_ACK 0xFB
//NG MESSAGE TO CONTROLLER // FE FE 70 E0 FA FD
#define CIV_B_NAK 0xFA  
//#define CIV_B_ 0x
//#define CIV_B_ 0x

/*
 *    FE FE   E0     42    04   00 01    FD       - LSB
 *    start target source cmd sub  data     end  
 *    bytes addr   addr       cmd  byte(s)   byte
 *    
 *    00 - mode
 *         eg. <FE FE E0 42 04 00 01 FD -LSB
 *    03 - frequency     
 *         eg. <FE FE E0 42 03 00 00 58 45 01 FD  -145.580.000
 */

typedef enum {
  CIV_STATE_IDLE,
  CIV_STATE_START, // expect 2nd start byte
  CIV_STATE_ADR_D, // expect destination address
  CIV_STATE_ADR_S, // expect source address
  CIV_STATE_DATA,  // expect cmd, sub and data byte(s) including terminator (CIV_B_END) 
  CIV_STATE_END, // expect end byte
  CIV_STATE_ERROR=-1
} civ_state_t ;


typedef enum {
  CIV_CMD_TR_FREQU = 0x00, // send frequency data (transceive)
  CIV_CMD_TR_MODE = 0x01, // send mode data (transceive)
  CIV_CMD_R_BAND_EDGE = 0x02, // read band edge frequencies
  CIV_CMD_R_OP_FREQU = 0x03, // read operating frequency
  CIV_CMD_R_OP_MODE = 0x04, // read operating mode
  CIV_CMD_S_OP_FREQU = 0x05, // set operating frequency
  CIV_CMD_S_OP_MODE = 0x06, // set operating mode
  CIV_CMD_S_VFO = 0x07, // select VFO mode
  CIV_CMD_S_MEM_MODE = 0x08, // select memory mode
  CIV_CMD_S_MEM = 0x09, // memory write
  CIV_CMD_S_MEM_VFO = 0x0A, // memory copy to VFO
  CIV_CMD_S_MEM_CLR = 0x0B, // memory clear
  CIV_CMD_R_OFFS = 0x0C, // read frequency offset
  CIV_CMD_S_OFFS = 0x0D, // send frequency offset
  CIV_CMD_C_SCAN = 0x0E, // initate scan
  CIV_CMD_MEM = 0x1A, // read/write memory 
//  CIV_CMD_ = 0x, //
//  CIV_CMD_ = 0x, //
//  CIV_CMD_ = 0x, //
//  CIV_CMD_,
} civ_cmd_t ;

class IcomCIV {

  public:
    // c-tor
    IcomCIV(HardwareSerial* civ_port, Serial_* con_port);
    
    inline void setMyAddress(byte addr) { _myAddress = addr; };
    inline byte getMyAddress(void) { return _myAddress; };
    inline byte getSrcAddress(void) { return record.adr_s; };
    inline byte getDstAddress(void) { return record.adr_d; };
    
    void setup(void);
    
    inline civ_state_t getState(void) { return _state; };
    void state_switch(civ_state_t st);
    
    civ_state_t update_ci(uint8_t ch);

    static void send(HardwareSerial* s, byte addr_from, byte addr_to, byte* buf, size_t len);
    static void send_nak(HardwareSerial* s, byte addr_from, byte addr_to);
    static void send_ack(HardwareSerial* s, byte addr_from, byte addr_to);

    typedef struct {
        uint8_t adr_s; // source address 
        uint8_t adr_d; // destination address
        uint8_t buf[CIV_BUFFER_SIZE];
    } civ_record_t;
    
    civ_record_t record;
    
protected:
    HardwareSerial *_civ_port; ///< serial port of CI-V
    /** 
     * serial port of console 
     * (for debug and command interface)
     */
    Serial_* _console;

    byte _myAddress; ///< my CI-V endpoint address
    civ_state_t _state; ///< stae of FSM
    
    byte _buf_idx; // incomming data buffer index
    
    bool civ_echo_enable = false;
    uint32_t civ_timout_val = 10000;
    uint32_t civ_timout_timer;

};
