/**
 * @file
 * 
 * @breaf Implement Icom CI-V protocol 
 * @author DG3QQ
 *
 * The goal of this is to act as a Icom CI-V protocol from the
 * radio CAT point of view.
 * 
 * This work was a need from my side for testing device drivers of CHIRP
 * project.
 *
 * This is a experimental/preliminary implementation of the protocol.
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

#include <Arduino.h>

#include "icom_civ.h"

/* ctor */
IcomCIV::IcomCIV(HardwareSerial* civ_port, Serial_* con_port) {
    _civ_port = civ_port;
    _console = con_port;
    _state = CIV_STATE_IDLE;
    
}

void IcomCIV::setup(void) {
  ;
}

/** send data  
 *  
 *  send raw data given in buf, terminated by CIV_B_END.
 *  
 * @param s - serial interface  
 * @param addr_from - source address 
 * @param addr_to - destination address
 * @param buf - data to send
 * @param len - how many to send (max); prevent runaway 
 */
void IcomCIV::send(HardwareSerial* s, byte addr_from, byte addr_to, byte* buf, size_t len) {
  uint8_t ch;
  size_t idx = 0;
  s->write(CIV_B_START);
  s->write(CIV_B_START);
  s->write(addr_to);
  s->write(addr_from);
    
  do {
     ch = buf[idx];
     s->write(ch); 
  } while ((ch != CIV_B_END) && (++idx<len));
  if ((ch != CIV_B_END)) {
    // FixMe: What should we do on mal terminated send buffer data  
    ;
  }
}

/** send negative acnolidge telegram (error message)   
 *  
 * @param s - serial interface  
 * @param addr_from - source address 
 * @param addr_to - destination address
 */
 void IcomCIV::send_nak(HardwareSerial* s, byte addr_from, byte addr_to) {
  s->write(CIV_B_START);
  s->write(CIV_B_START);
  s->write(addr_to);
  s->write(addr_from);
  s->write(CIV_B_NAK);
  s->write(CIV_B_END);
}

/** send positive acnolidge telegram   
 *  
 * @param s - serial interface  
 * @param addr_from - source address 
 * @param addr_to - destination address
 */
void IcomCIV::send_ack(HardwareSerial* s, byte addr_from, byte addr_to) {
  s->write(CIV_B_START);
  s->write(CIV_B_START);
  s->write(addr_to);
  s->write(addr_from);
  s->write(CIV_B_ACK);
  s->write(CIV_B_END);
}

/** perform a state transition of the FSM
 * @param st - new state ID   
 */  
void IcomCIV::state_switch(civ_state_t st) {
  _state = st;
  _console->print(st);
  //_console->println(PSTR(" switch"));
}
  
/** the heart of the protocol
 * 
 * run/update the FSM
 * 
 * @param ch - incomming data byte
 * @return current state of FSM   
 */  
civ_state_t IcomCIV::update_ci(uint8_t ch) {
    
  _civ_port->write(ch); //
  
  if (civ_echo_enable) {
    //CONSOLE_SERIAL.write(ch);
    _console->print(ch, HEX);
  }
  
  civ_timout_timer = millis(); civ_timout_val;
  switch(_state) {
    case CIV_STATE_IDLE: {
        if (CIV_B_START == ch) {
          state_switch(CIV_STATE_START);
        }
      } break;

    case CIV_STATE_START: { // expect 2nd start byte
        if (CIV_B_START == ch) {
          state_switch(CIV_STATE_ADR_D);
        } else { // got wrong byte; signal error 
          state_switch(CIV_STATE_ERROR);
        }
      } break;

    case CIV_STATE_ADR_D: { // expect destination address
        record.adr_d = ch;
        state_switch(CIV_STATE_ADR_S);
      } break;

    case CIV_STATE_ADR_S: { // expect source address
        record.adr_s = ch;
        _buf_idx =0;
        state_switch(CIV_STATE_DATA);
      } break;

    case CIV_STATE_DATA: { 
      // expect cmd, sub and data byte(s) including terminator (CIV_B_END) 
        if (_buf_idx < CIV_BUFFER_SIZE) {
          record.buf[_buf_idx++] = ch;
        } else { // runaway data; protect buffer overflow
          state_switch(CIV_STATE_ERROR);
        }
        if (CIV_B_END == ch) {
          state_switch(CIV_STATE_END);
        }
      } break;
#if 0
    case CIV_STATE_END: { // expect end byte
        print_buffer(&CONSOLE_SERIAL, civ_record.buf, sizeof(civ_record.buf));
        state_switch(CIV_STATE_IDLE);
      } break;

    case CIV_STATE_ERROR: {
        state_switch(CIV_STATE_IDLE);
      } break;
#endif
    default: {
        state_switch(CIV_STATE_ERROR);
      } break;
  }
  return _state;
}
