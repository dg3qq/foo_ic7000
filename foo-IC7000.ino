/**
 * @file
 * 
 * @brief emulation Icom IC7000 CI-V (CAT) side of radio
 * 
 * @author DG3QQ
 *
 * The goal of this is to act as a Icom IC7000 radio from the
 * CAT point of view.
 * 
 * This work was a need from my side for testing device drivers of CHIRP
 * project.
 *
 * This code has been built with the review of various sources:
 * - 
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
#include <EEPROM.h>

#include "icom_civ.h" // CI-V protocoll
#include "icom_ic7000.h" // radio properties
#include "cli.h" // command interface

#define BLINK_INTERVALL 500
uint32_t _timer;

 /*
 * # serial console/terminal command and debug output goes here
 */
#define CONSOLE_SERIAL Serial // arduino micro USB serial 
#define CONSOLE_BAUDRATE 9600  

/*
 * # 2-nd serial CI-V interface goes here
 */
#define CIV_SERIAL Serial1 // arduino micro hardware serial Tx/Rx
#define CIV_BAUDRATE ICOM_DEFAULT_CIV_BAUDRATE

char str_scratch_80[80];
char str_scratch_20[20];

#define RAMEM
#define N_BANKS 2 // 5
#define N_CHANS 10

mem_record_t EEMEM ee_radio_mem[N_BANKS][N_CHANS];
#ifdef RAMEM
mem_record_t radio_mem[N_BANKS][N_CHANS];
#endif
mem_record_t a_chan;

//civ_record_t civ_record;

// CI-V interface instance
IcomCIV civ_ci(&CIV_SERIAL, &CONSOLE_SERIAL);

// command line interface instance
CommandLineInterface cmd_ci(&CONSOLE_SERIAL);

/**
 * rudimentary implementatin of memory section dump (RAM) 
 * 
 * @param s - pointer to serial device class 
 * @param buf - pointer to the section to dump  
 * @param len - how manny
 */
void dump(Serial_* s, byte* buf, size_t len) {
  byte* p = buf;
  byte ch;
  size_t i = 0;
  do {
    ch = *p++;
      if (ch < 16) s->print("0");
      s->print(ch, HEX);
      s->print(" ");
      if ( 15== i %16) s->println();
    } while (++i < len);
    s->println();
}

/**
 * create 'test data' channal info record
 * 
 * @pram buf - pointer where to put the data to
 */
void test_data(mem_record_t* buf) {
  byte cd[] = {
    //0x01, // byte bnk = bank A (2=B, 3=C,...)
    //0x00, 0x01, // byte mn[2] = memmory 0001 ee_chan_mem();
    0x01, // byte sel
    0x00, 0x00, 0x80, 0x45, 0x01, // frqu1 - 145800.000 kHz 
    0x05, // byte mo1 - mode                                               .               
    0x01, // byte fi1 
    0x11, // byte fg1 TxSub, DUP+,  
    0x00, 0x08, 0x85, // byte s_tx1[3] = 88.5 
    0x00, 0x08, 0x85, // byte S_rx1[3]                                                .               
    0x00, 0x00, 0x23, // byte dcs_1[3]
    0x00, 0x00, 0x00, 0x44, 0x01, // byte freq2[5] 1440000000 
    0x05, // byte mo2
    0x01, // byte fi2
    0x00, // byte fg2
    0x00, 0x08, 0x85, //  byte s_tx2
    0x00, 0x08, 0x85, // byte s_rx2 [3]
    0x00, 0x00, 0x23, // byte dcs_2[3]
    'A', 'P', 'R', 'S', ' ', ' ', ' ', ' ', ' ',  // byte name[9] "APRS_____" 
  };
  memcpy((byte*)buf, cd, sizeof(cd));
}

/**
 * create empty channal info record
 * 
 * @pram buf - pointer where to put the data to 
 */
void empty_data(mem_record_t* buf) {
#if 01
  memset((byte*)buf, 0xFF, sizeof(mem_record_t));
  memset((byte*)buf->name, ' ', 9 /*sizeof(mem_record_t.name)*/);
#else
    byte cd[] = {
    //0x01, // byte bnk = bank A (2=B, 3=C,...)
    //0x00, 0x01, // byte mn[2] = memmory 0001 ee_chan_mem();
    0x01, // byte sel
    0x00, 0x00, 0x00, 0x00, 0x00, // frqu1 - 0 kHz 
    0x00, // byte mo1 - mode                                               .               
    0x00, // byte fi1 
    0x00, // byte fg1 TxSub, DUP+,  
    0x00, 0x08, 0x85, // byte s_tx1[3] = 88.5 
    0x00, 0x08, 0x85, // byte S_rx1[3]                                                .               
    0x00, 0x00, 0x23, // byte dcs_1[3]
    0x00, 0x00, 0x00, 0x00, 0x00, // byte freq2[5] 0 
    0x00, // byte mo2
    0x00, // byte fi2
    0x00, // byte fg2
    0x00, 0x08, 0x85, //  byte s_tx2
    0x00, 0x08, 0x85, // byte s_rx2 [3]
    0x00, 0x00, 0x23, // byte dcs_2[3]
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  // byte name[9] "APRS_____" 
  };
  memcpy((byte*)buf, cd, sizeof(cd));
#endif
}

/**
 * read channal data from memory
 * 
 *  @param bnk - memory bank 
 *  @param mn - channal number
 *  @param cd - pointer where to put the cannal data record (raw data)
 */
void read_chan_mem(mem_record_t* cd, int bnk, int mn) {
  int eeAddress = 0;
  
  //test_data((byte*) cd);
  eeAddress = bnk * mn * sizeof(mem_record_t);
#ifdef RAMEM
  memcpy((byte*)cd /*dst*/, (byte*)&radio_mem[bnk][mn] /*src*/, sizeof(mem_record_t));
#else  
  EEPROM.get(eeAddress, *cd);
  // ee_read_block(cd, &radio_mem[bnk][mn], sizeof(mem_record_t));
#endif
  //dump(&CONSOLE_SERIAL, (byte*)cd, sizeof(mem_record_t));
}   
     
/** 
 *  write data to memory 
 *  
 *  @param bnk - memory bank 
 *  @param mn - channal number
 *  @param cd - pointer where to get the cannal data record (raw data)
 */
void write_chan_mem(mem_record_t* cd, int bnk, int mn) {
  int eeAddress = 0;
  
  //dump(&CONSOLE_SERIAL, (byte*)cd, sizeof(mem_record_t));
  eeAddress = bnk * mn * sizeof(mem_record_t);
#ifdef RAMEM  
  memcpy((byte*)&radio_mem[bnk][mn] /*dst*/, (byte*)cd /*src*/, sizeof(mem_record_t));
#else
  EEPROM.put(eeAddress, *cd);
#endif
}        

/**
 * reset/clear channal memory to 'factory' defalults
 */
void reset_ee_chan_mem(void) {
  int eeAddress;
  
  empty_data(&a_chan);
  //test_data(&a_chan);
  //dump(&CONSOLE_SERIAL, (byte*)&a_chan, sizeof(mem_record_t));
  for(int bnk = 0; bnk<N_BANKS; ++bnk) {
    for(int mn =0; mn<N_CHANS; ++mn) {
      eeAddress = bnk * mn * sizeof(mem_record_t);
#ifdef RAMEM
      memcpy((byte*)&radio_mem[bnk][mn] /*dst*/, (byte*)&a_chan, sizeof(mem_record_t));
#else
      EEPROM.put(eeAddress, a_chan);
#endif
    }
  }
  //dump(&CONSOLE_SERIAL, (byte*)&radio_mem, sizeof(radio_mem));
}

/**
 * dump entire channal memory 
 */
void dump_ee_chan_mem(void) {
  int eeAddress = 0;
  
  empty_data(&a_chan);
  for(int bnk = 0; bnk<N_BANKS; ++bnk) {
    for(int mn =0; mn<N_CHANS; ++mn) {
      snprintf_P(str_scratch_80, 80, PSTR("== bnk=%i mn=%i =="), bnk, mn);
      CONSOLE_SERIAL.println(str_scratch_80);
      eeAddress = bnk * mn * sizeof(mem_record_t);
#if 0
      EEPROM.get(eeAddress, a_chan);
#else
      memcpy((byte*)&a_chan /*dst*/, (byte*)&radio_mem[bnk][mn], sizeof(mem_record_t));
#endif
      dump(&CONSOLE_SERIAL, (byte*)&a_chan, sizeof(mem_record_t));
    }
  }
}


/**
 * print/dump CI-V data buffer; 
 * 
 * Not confuse with @dump. This funktion honors the CI-V protocol telegram 
 * terminataor byte (\0xFD) and stops there after
 * 
 * @param s - pointer to output/stream device
 * @param buf - pointer to data to dump 
 * @param len - maximal length of dump in bytest; to prevent runnaway action if terminator not found. 
 */
void print_buffer(Serial_* s, byte* buf, size_t len) {
    s->print("<");
    for (size_t i = 0; i < len; i++) {
      if (buf[i] < 16) s->print("0");
      s->print(buf[i], HEX);
      s->print(" ");
      if (buf[i] == CIV_B_END) break;
    }
    s->println();
}

/**
 * print/dump CI-V data buffer in a slighty interpreted way. 
 * 
 * This will extract the to parties of the transaction 
 * (soure aod destinationspt) as well as the 'payload' body and 
 * print it in a more use friendly way
 * 
 * @param s - pointer to output/stream device
 * @param r - pointer to CI-V transaction data 
 */
void print_request(Serial_* s, IcomCIV::civ_record_t* r) {
  char cb[32];
  sprintf(cb, "from %02X ", r->adr_s);
  s->print(cb);
  sprintf(cb, "to %02X ", r->adr_d);
  s->print(cb);
  //s->print(PSTR(" to ")); s->print(r->adr_d);
  print_buffer(s, r->buf, CIV_BUFFER_SIZE);
}

/** 
 * convert from litle endian BCD (lowest two digits come first)
 * 
 * @param b - pointer to the byte sequenc
 * @param s - how many bytes to take into account 
 */
uint32_t _from_bcd_lite_endian(byte* b, uint8_t s) {
  uint32_t r = 0;
  byte* p=&b[s-1];

  for(int8_t i=s; i>0; i--) {
    r *= 100;
    r += (*p & 0x0F) + (*p>>4 & 0x0F)*10;
    p--;
  }
  return r;
}

/** 
 * convert from big endian BCD (highes two digits come first) 
 * 
 * @param b - pointer to the byte sequenc
 * @param s - how many bytes to take into account 
 */
uint32_t _from_bcd_big_endian(byte* b, uint8_t s) {
  uint32_t r = 0;
  byte* p=b;

  for(int8_t i=s; i>0; i--) {
    r *= 100;
    r += (*p & 0x0F) + (*p>>4 & 0x0F)*10;
    p++;
  }
  return r;
}

/**
 * convert/retrive/lookup  a mode ID to string 
 * 
 * 
 * @param m - te mode id to convert from
 * @param b - 'scratch' char buffer to sprintf to 
 * @return converted str; for ease of nesting it is essentialy the given buffer 
 * 
 * @note There is no 'bonb proofing' here! 
 *  Exceding values or what not will cause unpredicted behaviour.    
 */
char* _mode(char* buf, byte m) {
  const char* mode[] = {"LSB","USB","AM","CW","FSK","FM","WFM", "CW-R", "FSK-R" };
  sprintf(buf, mode[m]);
  return buf;  
}

//void _dcs()

/**
 * extract some parts of a channal info and print to console  
 * 
 * @param s - pointer to 'console' stream object
 * @param cm - pointer to channal memory record data  
 */
void print_ch_info(Serial_* s, mem_record_t* cm) {
  char cb[32];
  char str_d[12]; // room for 5 bytes -> 10 digits, decimal and \0 
  
  double f = (double)_from_bcd_lite_endian(cm->frqu1 ,5) / 1000;
  sprintf(cb, "F1=%s ", dtostrf(f, 10, 3, str_d));
  s->print(cb);

  sprintf(cb, "%s ", _mode(str_d, cm->mo1));
  s->print(cb);
  
  f = (double)_from_bcd_big_endian(cm->s_tx1, 3) / 10;
  sprintf(cb, "S_tx1=%s ", dtostrf(f, 4, 1, str_d));
  s->print(cb);

  f = (double)_from_bcd_big_endian(cm->s_rx1, 3) / 10;
  sprintf(cb, "S_rx1=%s ", dtostrf(f, 4, 1, str_d));
  s->print(cb);

  f = (double)_from_bcd_lite_endian(cm->frqu2 ,5) / 1000;
  sprintf(cb, "F2=%s ", dtostrf(f, 10, 3, str_d));
  s->print(cb);

  sprintf(cb, "%s ", _mode(str_d, cm->mo2));
  s->print(cb);
  
  f = (double)_from_bcd_big_endian(cm->s_tx2, 3) / 10;
  sprintf(cb, "S_tx2=%s ", dtostrf(f, 4, 1, str_d));
  s->print(cb);

  f = (double)_from_bcd_big_endian(cm->s_rx2, 3) / 10;
  sprintf(cb, "S_rx2=%s ", dtostrf(f, 4, 1, str_d));
  s->print(cb);

  
  memset(str_d, 0, sizeof(str_d));
  memcpy(str_d, cm->name, 9/*sizeof(cm->name)*/); 
  s->print(str_d);
  s->println();
}

/**
 * evaluate received CI-V command and take action on the request
 * 
 * @param r - pointer the telegram data   
 * 
 * @return result (0== success) other definitely an error.  
 * 
 * ToDo: ?does it make sense here to distinhuish here
 */
int civ_parse_command(IcomCIV::civ_record_t* r) {
  int res = -1;
  byte cmd, sub;
  cmd   = r->buf[0];
  sub = r->buf[1];
  if ((cmd == 0x1A) and (sub == 0x00)) {
    // r/w extended memory
    byte bnk = r->buf[2] -1; // bnk [1, 2, ..., N_BANKS] // bank (1->A, 2->B...)
    
    uint8_t mn; // memory number (2byte BCD)
    mn = (r->buf[4] & 0x0F) + (r->buf[4]>>4 & 0x0F) *10 + (r->buf[3] & 0x0F)*100; //  + (r->buf[4]>>4 & 0x0F)+1000
    mn -=1; // memory number [1, 2,] ????

#if 0
  bool off_range = false;
    (bnk>N_BANKS) || ;
      if off_range  {
        ; // out of range
        // civ_ci.send_nak(&CIV_SERIAL, r->adr_d /*addr_from*/, r->adr_s /*addr_to*/);
      }
      // no ][N_CHANS];
#endif
      
    if (r->buf[5] != CIV_B_END) {
      // write
      memcpy(&a_chan /*dst*/, &r->buf[5] /*src*/, sizeof(mem_record_t));
      print_ch_info(&CONSOLE_SERIAL, &a_chan);

      CONSOLE_SERIAL.print(F("write to mn= ")); CONSOLE_SERIAL.println(mn);  
      if ((bnk<N_BANKS) && (mn<N_CHANS)) {
        //dump(&CONSOLE_SERIAL, (byte*)&a_chan, sizeof(mem_record_t));
        write_chan_mem(&a_chan, bnk, mn);

      } else {
        ; // silently ignore      
      }
      // ToDo: return error message when out of range 
      civ_ci.send_ack(&CIV_SERIAL, r->adr_d /*addr_from*/, r->adr_s /*addr_to*/);
      res = 0;
    } else {
      // read

      if ((bnk<N_BANKS) && (mn<N_CHANS)) {
        read_chan_mem(&a_chan, bnk, mn);
      } else {
        // fill with empty data
        empty_data(&a_chan);
        //test_data((byte*) &a_chan);
      }

      if (a_chan.sel != 0xFF) {
        // our not empty
        // return memory content message
        memcpy(&r->buf[5] /*dst*/, &a_chan /*src*/, sizeof(mem_record_t));
        r->buf[5+sizeof(a_chan)] = CIV_B_END;
        civ_ci.send(&CIV_SERIAL, r->adr_d /*addr_from*/, r->adr_s /*addr_to*/, r->buf /**buf*/, CIV_BUFFER_SIZE /*len*/);
        res = 0;
      } else {
        // return empty memory message
        /*
         * empty/blank must return different message (see icom-civ.pdf section 5-3) 
         * 
         * @code:text 
         * FE FE E0 70 1A 00   01 00 11 FF FD
         *       ^  ^  ^  ^    ^  ^     ^
         *       !  !  !  !    !  !     ! blank indicator
         *       !  !  !  !    !  ! mn 0011    
         *       !  !  !  !    ! bnk (1=A)
         *       !  !  !  ! sub             
         *       !  !  ! cmd                     
         *       !  ! src 70= radio
         *       ! dst E0=control        
         * @end_code                                   
         */
        // the cmd, sub, bnk and mn data is still present 
        r->buf[5] = 0xFF; // CIV_B_EMPTY;
        r->buf[6] = CIV_B_END;
        civ_ci.send(&CIV_SERIAL, r->adr_d /*addr_from*/, r->adr_s /*addr_to*/, r->buf /**buf*/, CIV_BUFFER_SIZE /*len*/);
        res = 0;
      }
    }
  }
  return res;
}

#define MAX_ARGS 6
/** 
 * Quick /n dirty action to parsee/evaluate commands from 'Command Line Interface' control terminal  
 * 
 * callback function
 * 
 *  ToDo: make this smart; 
 *  ToDo: refactor tis out to seperata module!
 */
void con_ci_exec_parse_cb(char* con_buf) {
  char* argv[MAX_ARGS]; 
  char* token;

  int bnk = 0;
  int mn = 0;
  
  int8_t argc = 0;
 
  for (token = strtok(con_buf, " "); 
      token != NULL;
      token = strtok(NULL, " ")) {
    argv[argc] = token; 
    CONSOLE_SERIAL.println(argv[argc]);
    argc++;
    if (argc == MAX_ARGS) break;
  }
  //CONSOLE_SERIAL.println(argc);
  //CONSOLE_SERIAL.println(argv[0]);
  switch(argv[0][0]) {
    case 'c': {
        // clear whole channal memory
        CONSOLE_SERIAL.println(F("memory clear"));
        reset_ee_chan_mem();
      } break;
    case 'd': {
        // dump/display whole channal memory
        CONSOLE_SERIAL.println(F("memory dump"));
        //dump(&CONSOLE_SERIAL,  , byte* buf, size_t len)
        dump_ee_chan_mem();
    } break;
    case 'p': {
        // preset one channal memory with test data 
        if (argc==3) {
          bnk = atoi(argv[1]);
          mn = atoi(argv[2]);
        } else {
          mn = 0;
          bnk = 0;
        }
        
        CONSOLE_SERIAL.print(F("preset chanal "));
        snprintf_P(str_scratch_80, 80, PSTR("bnk=%i mn=%i"), bnk, mn);
        CONSOLE_SERIAL.println(str_scratch_80); 

        test_data(&a_chan);
        //dump(&CONSOLE_SERIAL, (byte*)&a_chan, sizeof(mem_record_t));
        write_chan_mem(&a_chan /*cd*/,  bnk, mn);
        read_chan_mem(&a_chan /*cd*/,  bnk, mn);
        print_ch_info(&CONSOLE_SERIAL, &a_chan);
        CONSOLE_SERIAL.println();
    } break;
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  while( !Serial ) ; // wait until the serial port gets available (micro) 
  CONSOLE_SERIAL.begin(CONSOLE_BAUDRATE);
  CIV_SERIAL.begin(CIV_BAUDRATE);
  CONSOLE_SERIAL.println("huhu");
  
  civ_ci.setup();
  civ_ci.setMyAddress(ICOM_DEFAULT_CIV_ADDRESS_IC7000);
  cmd_ci.setup(con_ci_exec_parse_cb); // set the command callback
#ifdef RAMEM
  // initialize radio memory (RAM data) 
  reset_ee_chan_mem();
  test_data(&a_chan);
  //dump(&CONSOLE_SERIAL, (byte*)&a_chan, sizeof(mem_record_t));
  write_chan_mem(&a_chan /*cd*/,  0, 0); // bank A, chn 1 is used
#endif 
  //dump_ee_chan_mem();
  //dump(&CONSOLE_SERIAL, (byte*)radio_mem, sizeof(radio_mem));
}

// the loop function runs over and over again forever
void loop() {
  if (millis() - _timer >= BLINK_INTERVALL) {
    _timer = millis(); 
    static bool st;
    st = st ^ true;
    digitalWrite(LED_BUILTIN, st);   // toggle the LED
    //CIV_SERIAL.write('a');
    //CONSOLE_SERIAL.write('b');     
  }
  if (CONSOLE_SERIAL.available()) {
    cmd_ci.ci_update(CONSOLE_SERIAL.read());
  }
  
  if (CIV_SERIAL.available()) {
    switch( civ_ci.update_ci(CIV_SERIAL.read()) ) {
      case CIV_STATE_END: { 
          CONSOLE_SERIAL.println("process");
          //print_buffer(&CONSOLE_SERIAL, civ_record.buf, sizeof(civ_record.buf));
          print_request(&CONSOLE_SERIAL, &civ_ci.record);
          civ_parse_command(&civ_ci.record);
          civ_ci.state_switch(CIV_STATE_IDLE);
        } break;
  
      case CIV_STATE_ERROR: {
          CONSOLE_SERIAL.println("ERROR");
          print_buffer(&CONSOLE_SERIAL, civ_ci.record.buf, CIV_BUFFER_SIZE /*sizeof(IcomCIV::civ_record_t.buf)*/);
          civ_ci.state_switch(CIV_STATE_IDLE);
          CONSOLE_SERIAL.println();
        } break;
    }
  }
}
