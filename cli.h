/**
 * @file
 * 
 * @breef Implement simple command dinterface 
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

#pragma once

#define CON_BUF_SIZE 40

/**
  * This is a most rudimentary command line input.   
*/
class CommandLineInterface {
  
  public:  
    /** c-tor
    * @param s - serial IO object
    */ 
    CommandLineInterface(Serial_* s);

    /**
     * parse and execute funtion prototype
     * 
     * @param cb - command buffer  
     */
    typedef void (*exec_cb_t)(const char* cb);
    
    /** setup function
     * 
     * pass callback
     *
     * @param cb_fn - callback function pointer
     */
    void setup(exec_cb_t cb_fn);
    
    /** the heart of the command interface
     *
     * 
     * @param s - serial IO object 
     * @param ch - incomming character
     */
    void ci_update(uint8_t ch);
    
    /** clear/reset buffer
     * 
     */  
    void buffer_clr(void);

    protected:
      Serial_* _ci_port; ///< serial line instance
      exec_cb_t _exec_cb; ///< callbeck function pointer
      char _buffer[CON_BUF_SIZE]; ///< input buffer
      int _buf_idx; ///< input buffer index
};
