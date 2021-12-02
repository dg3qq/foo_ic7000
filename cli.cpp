/**
 * @file
 * 
 * @brief Implement simple command dinterface 
 * @author DG3QQ
 *
 * This work was a need from my side for testing device drivers of CHIRP
 * project.
 *
 * This is a experimental/preliminary implementation of ...
 *
 * There meight be issues and deficiencies; not bomb proof yet.
 *
 * I can do better, but for the time beeing it does its job.  
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

#include "cli.h"

/* ctor */
CommandLineInterface::CommandLineInterface(Serial_* s) {
  _ci_port = s;
  _exec_cb =  NULL;  
  buffer_clr();
};

/** setup
 * 
 * @param cb_fn - callback function pointer    
 */  
void CommandLineInterface::setup(exec_cb_t cb_fn) {
  _exec_cb = cb_fn;
  buffer_clr();
}

/** clear/reset buffer
 * 
 */  
void CommandLineInterface::buffer_clr(void) {
  memset(_buffer, 0, CON_BUF_SIZE); // clear input buffer
  _buf_idx = 0; // input buffer index to start
}

#if 0
/**
 * @param s - serial interface  
*/
void civ::send_nak(Serial_* s, byte addr_from, byte addr_to) {
  s->write(CIV_B_START);
  s->write(CIV_B_START);
  s->write(addr_to);
  s->write(addr_from);
  s->write(CIV_B_NAK);
  s->write(CIV_B_END);
}
#endif

/** the heart of the protocol
 * 
 * run/update the FSM
 * 
 * read from command terminal serial line 
 * 
 * up to CON_BUF_SIZE ASCII-bytes (8bits)
 * This is a most rudimentary command line input.   
 * 
 * Input of UTF8 or other will crash!! 
 * 
 * We collect key strokes until (ASCII LF (\n)) 
 * 
 * When fund simple evaluation interpreter gets called 
 * 
 * This is a most rudimentary command line input.   
 * 
 * @param s - serial IO object 
 * @param ch - incomming data byte
 * @return current state of FSM   
 */
void CommandLineInterface::ci_update(uint8_t ch) {
    _ci_port->write(ch); // echo
    if (ch=='\n') {
      // encountered ENTER (ASCII LF)
      _ci_port->println();
      if (_exec_cb) _exec_cb(_buffer);    
      buffer_clr(); // reset CLI
    } else {
    
      _buffer[_buf_idx] = ch;
      if (++_buf_idx>CON_BUF_SIZE) {
        _ci_port->write('\x07'); // make noise (0x07 - ASCII BELL)
        _buf_idx--;       
      }
    }
}
