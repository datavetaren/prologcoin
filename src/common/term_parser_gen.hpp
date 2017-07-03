namespace prologcoin { namespace common { 

 enum symbol_t {
  SYMBOL_UNKNOWN = 0,
  SYMBOL_ARGUMENTS = 1,
  SYMBOL_ATOM = 2,
  SYMBOL_CONSTANT = 3,
  SYMBOL_FUNCTOR_LPAREN = 4,
  SYMBOL_LIST = 5,
  SYMBOL_LISTEXPR = 6,
  SYMBOL_NUMBER = 7,
  SYMBOL_SIGN = 8,
  SYMBOL_SUBTERM_1000 = 9,
  SYMBOL_SUBTERM_1200 = 10,
  SYMBOL_SUBTERM_999 = 11,
  SYMBOL_TERM_0 = 12,
  SYMBOL_TERM_1000 = 13,
  SYMBOL_TERM_1200 = 14,
  SYMBOL_TERM_999 = 15,
  SYMBOL_UNSIGNED_NUMBER = 16,
  SYMBOL_BIN_OP_1000 = 1000,
  SYMBOL_BIN_OP_1200 = 1001,
  SYMBOL_BIN_OP_999 = 1002,
  SYMBOL_COMMA = 1003,
  SYMBOL_FULL_STOP = 1004,
  SYMBOL_INF = 1005,
  SYMBOL_LBRACE = 1006,
  SYMBOL_LBRACKET = 1007,
  SYMBOL_LPAREN = 1008,
  SYMBOL_MINUS = 1009,
  SYMBOL_NAME = 1010,
  SYMBOL_NAME_LPAREN = 1011,
  SYMBOL_NAN = 1012,
  SYMBOL_NATURAL_NUMBER = 1013,
  SYMBOL_PLUS = 1014,
  SYMBOL_RBRACE = 1015,
  SYMBOL_RBRACKET = 1016,
  SYMBOL_RPAREN = 1017,
  SYMBOL_STRING = 1018,
  SYMBOL_UNARY_OP_1000 = 1019,
  SYMBOL_UNARY_OP_1200 = 1020,
  SYMBOL_UNARY_OP_999 = 1021,
  SYMBOL_UNSIGNED_FLOAT = 1022,
  SYMBOL_VARIABLE = 1023,
  SYMBOL_VBAR = 1024
 };

template<typename Base, typename... Args> class term_parser_gen : public Base {
 protected:

 term_parser_gen(Args&... args) : Base(args...) { }

 void process_state() {
  switch (Base::current_state()) {
   case 0: state0(); break;
   case 1: state1(); break;
   case 2: state2(); break;
   case 3: state3(); break;
   case 4: state4(); break;
   case 5: state5(); break;
   case 6: state6(); break;
   case 7: state7(); break;
   case 8: state8(); break;
   case 9: state9(); break;
   case 10: state10(); break;
   case 11: state11(); break;
   case 12: state12(); break;
   case 13: state13(); break;
   case 14: state14(); break;
   case 15: state15(); break;
   case 16: state16(); break;
   case 17: state17(); break;
   case 18: state18(); break;
   case 19: state19(); break;
   case 20: state20(); break;
   case 21: state21(); break;
   case 22: state22(); break;
   case 23: state23(); break;
   case 24: state24(); break;
   case 25: state25(); break;
   case 26: state26(); break;
   case 27: state27(); break;
   case 28: state28(); break;
   case 29: state29(); break;
   case 30: state30(); break;
   case 31: state31(); break;
   case 32: state32(); break;
   case 33: state33(); break;
   case 34: state34(); break;
   case 35: state35(); break;
   case 36: state36(); break;
   case 37: state37(); break;
   case 38: state38(); break;
   case 39: state39(); break;
   case 40: state40(); break;
   case 41: state41(); break;
   case 42: state42(); break;
   case 43: state43(); break;
   case 44: state44(); break;
   case 45: state45(); break;
   case 46: state46(); break;
   case 47: state47(); break;
   case 48: state48(); break;
   case 49: state49(); break;
   case 50: state50(); break;
   case 51: state51(); break;
   case 52: state52(); break;
   case 53: state53(); break;
   case 54: state54(); break;
   case 55: state55(); break;
   case 56: state56(); break;
   case 57: state57(); break;
   case 58: state58(); break;
   case 59: state59(); break;
   case 60: state60(); break;
   case 61: state61(); break;
   case 62: state62(); break;
   case 63: state63(); break;
   case 64: state64(); break;
   case 65: state65(); break;
   case 66: state66(); break;
   case 67: state67(); break;
   case 68: state68(); break;
   case 69: state69(); break;
   case 70: state70(); break;
   case 71: state71(); break;
   case 72: state72(); break;
   case 73: state73(); break;
   case 74: state74(); break;
   case 75: state75(); break;
   case 76: state76(); break;
   case 77: state77(); break;
   case 78: state78(); break;
   case 79: state79(); break;
   case 80: state80(); break;
   case 81: state81(); break;
   case 82: state82(); break;
   case 83: state83(); break;
   case 84: state84(); break;
   case 85: state85(); break;
   case 86: state86(); break;
   case 87: state87(); break;
   case 88: state88(); break;
   case 89: state89(); break;
   case 90: state90(); break;
   case 91: state91(); break;
   case 92: state92(); break;
   case 93: state93(); break;
   case 94: state94(); break;
   case 95: state95(); break;
   case 96: state96(); break;
   case 97: state97(); break;
   case 98: state98(); break;
   case 99: state99(); break;
   case 100: state100(); break;
   case 101: state101(); break;
   case 102: state102(); break;
   case 103: state103(); break;
   case 104: state104(); break;
   case 105: state105(); break;
   case 106: state106(); break;
   case 107: state107(); break;
   case 108: state108(); break;
   case 109: state109(); break;
   case 110: state110(); break;
   case 111: state111(); break;
   case 112: state112(); break;
   case 113: state113(); break;
   case 114: state114(); break;
   case 115: state115(); break;
   case 116: state116(); break;
   case 117: state117(); break;
   case 118: state118(); break;
   case 119: state119(); break;
   case 120: state120(); break;
   case 121: state121(); break;
   case 122: state122(); break;
   case 123: state123(); break;
   case 124: state124(); break;
   case 125: state125(); break;
   case 126: state126(); break;
   case 127: state127(); break;
   case 128: state128(); break;
   case 129: state129(); break;
   case 130: state130(); break;
   case 131: state131(); break;
   case 132: state132(); break;
   case 133: state133(); break;
   case 134: state134(); break;
   case 135: state135(); break;
   case 136: state136(); break;
   case 137: state137(); break;
   case 138: state138(); break;
   case 139: state139(); break;
   case 140: state140(); break;
   case 141: state141(); break;
   case 142: state142(); break;
   case 143: state143(); break;
   case 144: state144(); break;
   case 145: state145(); break;
   case 146: state146(); break;
   case 147: state147(); break;
   case 148: state148(); break;
   case 149: state149(); break;
   case 150: state150(); break;
   case 151: state151(); break;
   case 152: state152(); break;
   case 153: state153(); break;
   case 154: state154(); break;
   case 155: state155(); break;
   case 156: state156(); break;
   case 157: state157(); break;
   case 158: state158(); break;
   case 159: state159(); break;
   case 160: state160(); break;
   case 161: state161(); break;
   case 162: state162(); break;
   case 163: state163(); break;
   case 164: state164(); break;
   case 165: state165(); break;
   case 166: state166(); break;
   case 167: state167(); break;
   case 168: state168(); break;
   case 169: state169(); break;
   case 170: state170(); break;
   case 171: state171(); break;
   case 172: state172(); break;
   case 173: state173(); break;
   case 174: state174(); break;
   case 175: state175(); break;
   case 176: state176(); break;
   case 177: state177(); break;
   case 178: state178(); break;
   case 179: state179(); break;
   case 180: state180(); break;
   case 181: state181(); break;
   case 182: state182(); break;
   case 183: state183(); break;
   case 184: state184(); break;
   case 185: state185(); break;
   case 186: state186(); break;
   case 187: state187(); break;
   case 188: state188(); break;
   case 189: state189(); break;
   case 190: state190(); break;
   case 191: state191(); break;
   case 192: state192(); break;
   case 193: state193(); break;
   case 194: state194(); break;
   case 195: state195(); break;
   case 196: state196(); break;
   case 197: state197(); break;
   case 198: state198(); break;
   case 199: state199(); break;
   case 200: state200(); break;
   case 201: state201(); break;
   case 202: state202(); break;
   case 203: state203(); break;
   case 204: state204(); break;
   case 205: state205(); break;
   case 206: state206(); break;
   case 207: state207(); break;
   case 208: state208(); break;
   case 209: state209(); break;
   case 210: state210(); break;
   case 211: state211(); break;
   case 212: state212(); break;
   case 213: state213(); break;
   case 214: state214(); break;
   case 215: state215(); break;
   case 216: state216(); break;
   case 217: state217(); break;
   case 218: state218(); break;
   case 219: state219(); break;
   case 220: state220(); break;
   case 221: state221(); break;
   case 222: state222(); break;
   case 223: state223(); break;
   case 224: state224(); break;
   case 225: state225(); break;
   case 226: state226(); break;
   case 227: state227(); break;
   case 228: state228(); break;
   case 229: state229(); break;
   case 230: state230(); break;
   case 231: state231(); break;
   case 232: state232(); break;
   case 233: state233(); break;
   case 234: state234(); break;
   case 235: state235(); break;
   case 236: state236(); break;
   case 237: state237(); break;
   case 238: state238(); break;
   case 239: state239(); break;
   case 240: state240(); break;
   case 241: state241(); break;
   case 242: state242(); break;
   case 243: state243(); break;
   case 244: state244(); break;
   case 245: state245(); break;
   case 246: state246(); break;
   case 247: state247(); break;
   case 248: state248(); break;
   case 249: state249(); break;
   case 250: state250(); break;
   case 251: state251(); break;
   case 252: state252(); break;
   case 253: state253(); break;
   case 254: state254(); break;
   case 255: state255(); break;
   case 256: state256(); break;
   case 257: state257(); break;
   case 258: state258(); break;
   case 259: state259(); break;
   case 260: state260(); break;
   case 261: state261(); break;
   case 262: state262(); break;
   case 263: state263(); break;
   case 264: state264(); break;
   case 265: state265(); break;
   case 266: state266(); break;
   case 267: state267(); break;
   case 268: state268(); break;
   case 269: state269(); break;
   case 270: state270(); break;
   case 271: state271(); break;
   case 272: state272(); break;
   case 273: state273(); break;
   case 274: state274(); break;
   case 275: state275(); break;
   case 276: state276(); break;
   case 277: state277(); break;
   case 278: state278(); break;
   case 279: state279(); break;
   case 280: state280(); break;
   case 281: state281(); break;
   case 282: state282(); break;
   case 283: state283(); break;
   case 284: state284(); break;
   case 285: state285(); break;
   case 286: state286(); break;
   case 287: state287(); break;
   case 288: state288(); break;
   case 289: state289(); break;
   case 290: state290(); break;
   case 291: state291(); break;
   case 292: state292(); break;
   case 293: state293(); break;
   case 294: state294(); break;
   case 295: state295(); break;
   case 296: state296(); break;
   case 297: state297(); break;
   case 298: state298(); break;
   case 299: state299(); break;
   case 300: state300(); break;
   case 301: state301(); break;
   case 302: state302(); break;
   case 303: state303(); break;
   case 304: state304(); break;
   case 305: state305(); break;
   case 306: state306(); break;
   case 307: state307(); break;
   case 308: state308(); break;
   case 309: state309(); break;
   case 310: state310(); break;
   case 311: state311(); break;
   case 312: state312(); break;
   case 313: state313(); break;
   case 314: state314(); break;
   case 315: state315(); break;
   case 316: state316(); break;
   case 317: state317(); break;
   case 318: state318(); break;
   case 319: state319(); break;
   case 320: state320(); break;
   case 321: state321(); break;
   case 322: state322(); break;
   case 323: state323(); break;
   case 324: state324(); break;
  } }

 void state0() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(10); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(11); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(21); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(22); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(23); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(24); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(25); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state1() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_atom__name(Base::args(1)));
    break;
  }
 }

 void state2() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__atom(Base::args(1)));
    break;
  }
 }

 void state3() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__number(Base::args(1)));
    break;
  }
 }

 void state4() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_LBRACE:
   case SYMBOL_LBRACKET:
   case SYMBOL_LPAREN:
   case SYMBOL_MINUS:
   case SYMBOL_NAME:
   case SYMBOL_NAME_LPAREN:
   case SYMBOL_NATURAL_NUMBER:
   case SYMBOL_PLUS:
   case SYMBOL_STRING:
   case SYMBOL_UNARY_OP_999:
   case SYMBOL_UNSIGNED_FLOAT:
   case SYMBOL_VARIABLE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_functor_lparen__name_lparen(Base::args(1)));
    break;
  }
 }

 void state5() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(33); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(34); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state6() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF: Base::shift_and_goto_state(270); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(271); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(272); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state7() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__unsigned_number(Base::args(1)));
    break;
  }
 }

 void state8() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF:
   case SYMBOL_NAN:
   case SYMBOL_NATURAL_NUMBER:
   case SYMBOL_UNSIGNED_FLOAT:
    Base::push(Base::lookahead().ordinal(), Base::reduce_sign__minus(Base::args(1)));
    break;
  }
 }

 void state9() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF:
   case SYMBOL_NAN:
   case SYMBOL_NATURAL_NUMBER:
   case SYMBOL_UNSIGNED_FLOAT:
    Base::push(Base::lookahead().ordinal(), Base::reduce_sign__plus(Base::args(1)));
    break;
  }
 }

 void state10() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1200__term_1000(Base::args(1)));
    break;
   case SYMBOL_BIN_OP_1000:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
  }
 }

 void state11() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1200__term_1200(Base::args(1)));
    break;
  }
 }

 void state12() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
  }
 }

 void state13() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_BIN_OP_1000:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
  }
 }

 void state14() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__constant(Base::args(1)));
    break;
  }
 }

 void state15() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(273); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state16() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(275); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state17() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__list(Base::args(1)));
    break;
  }
 }

 void state18() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(277); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state19() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__string(Base::args(1)));
    break;
  }
 }

 void state20() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__variable(Base::args(1)));
    break;
  }
 }

 void state21() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000: Base::shift_and_goto_state(279); break;
  }
 }

 void state22() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(280); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(281); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(22); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(319); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(283); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(284); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state23() {
  switch (lookahead().ordinal()) {
   case SYMBOL_FULL_STOP: Base::shift_and_goto_state(320); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(321); break;
  }
 }

 void state24() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(10); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(11); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(21); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(22); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(24); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(323); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(25); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state25() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(285); break;
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(183); break;
  }
 }

 void state26() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(184); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(185); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(186); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(187); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(188); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(189); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(190); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(191); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(192); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(193); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(194); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(195); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(196); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(197); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(198); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(324); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(200); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(201); break;
  }
 }

 void state27() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
  }
 }

 void state28() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
  }
 }

 void state29() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_atom__name(Base::args(1)));
    break;
  }
 }

 void state30() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__atom(Base::args(1)));
    break;
  }
 }

 void state31() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__number(Base::args(1)));
    break;
  }
 }

 void state32() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(50); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(51); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state33() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(53); break;
  }
 }

 void state34() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
  }
 }

 void state35() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF: Base::shift_and_goto_state(54); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(55); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(56); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state36() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__unsigned_number(Base::args(1)));
    break;
  }
 }

 void state37() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
  }
 }

 void state38() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
  }
 }

 void state39() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__constant(Base::args(1)));
    break;
  }
 }

 void state40() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(72); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state41() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(226); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state42() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__list(Base::args(1)));
    break;
  }
 }

 void state43() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(228); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state44() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__string(Base::args(1)));
    break;
  }
 }

 void state45() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__variable(Base::args(1)));
    break;
  }
 }

 void state46() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(230); break;
   case SYMBOL_VBAR: Base::shift_and_goto_state(231); break;
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(232); break;
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_listexpr__subterm_999(Base::args(1)));
    break;
  }
 }

 void state47() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(269); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state48() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
  }
 }

 void state49() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
  }
 }

 void state50() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(52); break;
  }
 }

 void state51() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
  }
 }

 void state52() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
  }
 }

 void state53() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
  }
 }

 void state54() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_inf(Base::args(2)));
    break;
  }
 }

 void state55() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_nan(Base::args(2)));
    break;
  }
 }

 void state56() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_unsigned_number(Base::args(2)));
    break;
  }
 }

 void state57() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_atom__name(Base::args(1)));
    break;
  }
 }

 void state58() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__atom(Base::args(1)));
    break;
  }
 }

 void state59() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__number(Base::args(1)));
    break;
  }
 }

 void state60() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(77); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(78); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state61() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF: Base::shift_and_goto_state(80); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(81); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(82); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state62() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__unsigned_number(Base::args(1)));
    break;
  }
 }

 void state63() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
  }
 }

 void state64() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
  }
 }

 void state65() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__constant(Base::args(1)));
    break;
  }
 }

 void state66() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(83); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state67() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(89); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state68() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__list(Base::args(1)));
    break;
  }
 }

 void state69() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(218); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state70() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__string(Base::args(1)));
    break;
  }
 }

 void state71() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__variable(Base::args(1)));
    break;
  }
 }

 void state72() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(220); break;
  }
 }

 void state73() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(221); break;
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(222); break;
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_arguments__subterm_999(Base::args(1)));
    break;
  }
 }

 void state74() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(225); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state75() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
  }
 }

 void state76() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
  }
 }

 void state77() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(79); break;
  }
 }

 void state78() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
  }
 }

 void state79() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
  }
 }

 void state80() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_inf(Base::args(2)));
    break;
  }
 }

 void state81() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_nan(Base::args(2)));
    break;
  }
 }

 void state82() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_unsigned_number(Base::args(2)));
    break;
  }
 }

 void state83() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(84); break;
  }
 }

 void state84() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
  }
 }

 void state85() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1200__term_1000(Base::args(1)));
    break;
   case SYMBOL_BIN_OP_1000:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
  }
 }

 void state86() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1200__term_1200(Base::args(1)));
    break;
  }
 }

 void state87() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000: Base::shift_and_goto_state(92); break;
  }
 }

 void state88() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(93); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(94); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(215); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(96); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(97); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state89() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(216); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(129); break;
  }
 }

 void state90() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(217); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state91() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(98); break;
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(183); break;
  }
 }

 void state92() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(93); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(94); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(95); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(96); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(97); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state93() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
  }
 }

 void state94() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
  }
 }

 void state95() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__subterm_1000_bin_op_1000_subterm_1000(Base::args(3)));
    break;
  }
 }

 void state96() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(98); break;
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(99); break;
  }
 }

 void state97() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(101); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(102); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(104); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(105); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(106); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(107); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(108); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(109); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(110); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(111); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(112); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(113); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(114); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(115); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(97); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(214); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(117); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(118); break;
  }
 }

 void state98() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(93); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(94); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(100); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(96); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(97); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state99() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(101); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(102); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(104); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(105); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(106); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(107); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(108); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(109); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(110); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(111); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(112); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(113); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(114); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(115); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(97); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(116); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(117); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(118); break;
  }
 }

 void state100() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__subterm_999_comma_subterm_1000(Base::args(3)));
    break;
  }
 }

 void state101() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_atom__name(Base::args(1)));
    break;
  }
 }

 void state102() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__atom(Base::args(1)));
    break;
  }
 }

 void state103() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__number(Base::args(1)));
    break;
  }
 }

 void state104() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(119); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(120); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state105() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF: Base::shift_and_goto_state(122); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(123); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(117); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(118); break;
  }
 }

 void state106() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__unsigned_number(Base::args(1)));
    break;
  }
 }

 void state107() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
  }
 }

 void state108() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
  }
 }

 void state109() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__constant(Base::args(1)));
    break;
  }
 }

 void state110() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(125); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state111() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(127); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state112() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__list(Base::args(1)));
    break;
  }
 }

 void state113() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(135); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state114() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__string(Base::args(1)));
    break;
  }
 }

 void state115() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__variable(Base::args(1)));
    break;
  }
 }

 void state116() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__subterm_999_bin_op_999_subterm_999(Base::args(3)));
    break;
  }
 }

 void state117() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
  }
 }

 void state118() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
  }
 }

 void state119() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(121); break;
  }
 }

 void state120() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
  }
 }

 void state121() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
  }
 }

 void state122() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_inf(Base::args(2)));
    break;
  }
 }

 void state123() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_nan(Base::args(2)));
    break;
  }
 }

 void state124() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_unsigned_number(Base::args(2)));
    break;
  }
 }

 void state125() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(126); break;
  }
 }

 void state126() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
  }
 }

 void state127() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(128); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(129); break;
  }
 }

 void state128() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
  }
 }

 void state129() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(130); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state130() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1200__subterm_1200_bin_op_1200_subterm_1200(Base::args(3)));
    break;
  }
 }

 void state131() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1200__term_1000(Base::args(1)));
    break;
   case SYMBOL_BIN_OP_1000:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
  }
 }

 void state132() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1200__term_1200(Base::args(1)));
    break;
  }
 }

 void state133() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000: Base::shift_and_goto_state(138); break;
  }
 }

 void state134() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(139); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(140); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(180); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(142); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(143); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state135() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(181); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(177); break;
  }
 }

 void state136() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(182); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state137() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(144); break;
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(183); break;
  }
 }

 void state138() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(139); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(140); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(141); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(142); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(143); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state139() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
  }
 }

 void state140() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
  }
 }

 void state141() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__subterm_1000_bin_op_1000_subterm_1000(Base::args(3)));
    break;
  }
 }

 void state142() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(144); break;
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(145); break;
  }
 }

 void state143() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(147); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(148); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(149); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(150); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(151); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(152); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(153); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(154); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(155); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(156); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(157); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(158); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(160); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(161); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(143); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(179); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(164); break;
  }
 }

 void state144() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(139); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(140); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(146); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(142); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(143); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state145() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(147); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(148); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(149); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(150); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(151); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(152); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(153); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(154); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(155); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(156); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(157); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(158); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(160); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(161); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(143); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(162); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(164); break;
  }
 }

 void state146() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__subterm_999_comma_subterm_1000(Base::args(3)));
    break;
  }
 }

 void state147() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_atom__name(Base::args(1)));
    break;
  }
 }

 void state148() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__atom(Base::args(1)));
    break;
  }
 }

 void state149() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__number(Base::args(1)));
    break;
  }
 }

 void state150() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(165); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(166); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state151() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF: Base::shift_and_goto_state(168); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(169); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(164); break;
  }
 }

 void state152() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__unsigned_number(Base::args(1)));
    break;
  }
 }

 void state153() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
  }
 }

 void state154() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
  }
 }

 void state155() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__constant(Base::args(1)));
    break;
  }
 }

 void state156() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(171); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state157() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(173); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state158() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__list(Base::args(1)));
    break;
  }
 }

 void state159() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(175); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state160() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__string(Base::args(1)));
    break;
  }
 }

 void state161() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__variable(Base::args(1)));
    break;
  }
 }

 void state162() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__subterm_999_bin_op_999_subterm_999(Base::args(3)));
    break;
  }
 }

 void state163() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
  }
 }

 void state164() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
  }
 }

 void state165() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(167); break;
  }
 }

 void state166() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
  }
 }

 void state167() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
  }
 }

 void state168() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_inf(Base::args(2)));
    break;
  }
 }

 void state169() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_nan(Base::args(2)));
    break;
  }
 }

 void state170() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_unsigned_number(Base::args(2)));
    break;
  }
 }

 void state171() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(172); break;
  }
 }

 void state172() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
  }
 }

 void state173() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(174); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(129); break;
  }
 }

 void state174() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
  }
 }

 void state175() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(176); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(177); break;
  }
 }

 void state176() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
  }
 }

 void state177() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(178); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state178() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1200__subterm_1200_bin_op_1200_subterm_1200(Base::args(3)));
    break;
  }
 }

 void state179() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__unary_op_999_subterm_999(Base::args(2)));
    break;
  }
 }

 void state180() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__unary_op_1000_subterm_1000(Base::args(2)));
    break;
  }
 }

 void state181() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
  }
 }

 void state182() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1200__unary_op_1200_subterm_1200(Base::args(2)));
    break;
  }
 }

 void state183() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(184); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(185); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(186); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(187); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(188); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(189); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(190); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(191); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(192); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(193); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(194); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(195); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(196); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(197); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(198); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(199); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(200); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(201); break;
  }
 }

 void state184() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_atom__name(Base::args(1)));
    break;
  }
 }

 void state185() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__atom(Base::args(1)));
    break;
  }
 }

 void state186() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__number(Base::args(1)));
    break;
  }
 }

 void state187() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(202); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(203); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state188() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF: Base::shift_and_goto_state(205); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(206); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(207); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(200); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(201); break;
  }
 }

 void state189() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__unsigned_number(Base::args(1)));
    break;
  }
 }

 void state190() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
  }
 }

 void state191() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
  }
 }

 void state192() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__constant(Base::args(1)));
    break;
  }
 }

 void state193() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(208); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state194() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(210); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state195() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__list(Base::args(1)));
    break;
  }
 }

 void state196() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(212); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state197() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__string(Base::args(1)));
    break;
  }
 }

 void state198() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__variable(Base::args(1)));
    break;
  }
 }

 void state199() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__subterm_999_bin_op_999_subterm_999(Base::args(3)));
    break;
  }
 }

 void state200() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
  }
 }

 void state201() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
  }
 }

 void state202() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(204); break;
  }
 }

 void state203() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
  }
 }

 void state204() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
  }
 }

 void state205() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_inf(Base::args(2)));
    break;
  }
 }

 void state206() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_nan(Base::args(2)));
    break;
  }
 }

 void state207() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_unsigned_number(Base::args(2)));
    break;
  }
 }

 void state208() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(209); break;
  }
 }

 void state209() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
  }
 }

 void state210() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(211); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(129); break;
  }
 }

 void state211() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
  }
 }

 void state212() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(213); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(177); break;
  }
 }

 void state213() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
  }
 }

 void state214() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__unary_op_999_subterm_999(Base::args(2)));
    break;
  }
 }

 void state215() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__unary_op_1000_subterm_1000(Base::args(2)));
    break;
  }
 }

 void state216() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
  }
 }

 void state217() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_RBRACE:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1200__unary_op_1200_subterm_1200(Base::args(2)));
    break;
  }
 }

 void state218() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(219); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(177); break;
  }
 }

 void state219() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
  }
 }

 void state220() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
  }
 }

 void state221() {
  switch (lookahead().ordinal()) {
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(223); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state222() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(224); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state223() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_arguments__subterm_999_comma_arguments(Base::args(3)));
    break;
  }
 }

 void state224() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__subterm_999_bin_op_999_subterm_999(Base::args(3)));
    break;
  }
 }

 void state225() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__unary_op_999_subterm_999(Base::args(2)));
    break;
  }
 }

 void state226() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(227); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(129); break;
  }
 }

 void state227() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
  }
 }

 void state228() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(229); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(177); break;
  }
 }

 void state229() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
  }
 }

 void state230() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(233); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state231() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(234); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(235); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(236); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(237); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(238); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(239); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(240); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(241); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(242); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(243); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(244); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(245); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(246); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(247); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(248); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(249); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(250); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(251); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(252); break;
  }
 }

 void state232() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(268); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state233() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_listexpr__subterm_999_comma_listexpr(Base::args(3)));
    break;
  }
 }

 void state234() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_atom__name(Base::args(1)));
    break;
  }
 }

 void state235() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__atom(Base::args(1)));
    break;
  }
 }

 void state236() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__number(Base::args(1)));
    break;
  }
 }

 void state237() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(253); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(254); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state238() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF: Base::shift_and_goto_state(256); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(257); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(258); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(251); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(252); break;
  }
 }

 void state239() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__unsigned_number(Base::args(1)));
    break;
  }
 }

 void state240() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
  }
 }

 void state241() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
  }
 }

 void state242() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__constant(Base::args(1)));
    break;
  }
 }

 void state243() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(259); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state244() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(261); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state245() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__list(Base::args(1)));
    break;
  }
 }

 void state246() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(263); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state247() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__string(Base::args(1)));
    break;
  }
 }

 void state248() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__variable(Base::args(1)));
    break;
  }
 }

 void state249() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(265); break;
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_listexpr__subterm_999_vbar_subterm_999(Base::args(3)));
    break;
  }
 }

 void state250() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(234); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(235); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(236); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(237); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(238); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(239); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(240); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(241); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(242); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(243); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(244); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(245); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(246); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(247); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(248); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(250); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(267); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(251); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(252); break;
  }
 }

 void state251() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
  }
 }

 void state252() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
  }
 }

 void state253() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(255); break;
  }
 }

 void state254() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
  }
 }

 void state255() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
  }
 }

 void state256() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_inf(Base::args(2)));
    break;
  }
 }

 void state257() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_nan(Base::args(2)));
    break;
  }
 }

 void state258() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_unsigned_number(Base::args(2)));
    break;
  }
 }

 void state259() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(260); break;
  }
 }

 void state260() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
  }
 }

 void state261() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(262); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(129); break;
  }
 }

 void state262() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
  }
 }

 void state263() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(264); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(177); break;
  }
 }

 void state264() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
  }
 }

 void state265() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(234); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(235); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(236); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(237); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(238); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(239); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(240); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(241); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(242); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(243); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(244); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(245); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(246); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(247); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(248); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(250); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(266); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(251); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(252); break;
  }
 }

 void state266() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__subterm_999_bin_op_999_subterm_999(Base::args(3)));
    break;
  }
 }

 void state267() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_RBRACKET:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__unary_op_999_subterm_999(Base::args(2)));
    break;
  }
 }

 void state268() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__subterm_999_bin_op_999_subterm_999(Base::args(3)));
    break;
  }
 }

 void state269() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__unary_op_999_subterm_999(Base::args(2)));
    break;
  }
 }

 void state270() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_inf(Base::args(2)));
    break;
  }
 }

 void state271() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_nan(Base::args(2)));
    break;
  }
 }

 void state272() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_unsigned_number(Base::args(2)));
    break;
  }
 }

 void state273() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(274); break;
  }
 }

 void state274() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
  }
 }

 void state275() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(276); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(129); break;
  }
 }

 void state276() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
  }
 }

 void state277() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(278); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(177); break;
  }
 }

 void state278() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
  }
 }

 void state279() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(280); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(281); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(22); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(282); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(283); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(284); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state280() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
  }
 }

 void state281() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
  }
 }

 void state282() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__subterm_1000_bin_op_1000_subterm_1000(Base::args(3)));
    break;
  }
 }

 void state283() {
  switch (lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(285); break;
   case SYMBOL_BIN_OP_999: Base::shift_and_goto_state(286); break;
  }
 }

 void state284() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(288); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(289); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(290); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(291); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(292); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(293); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(294); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(295); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(296); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(297); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(298); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(299); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(300); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(301); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(302); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(284); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(318); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(304); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(305); break;
  }
 }

 void state285() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(280); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(281); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(22); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(287); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(283); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(284); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state286() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(288); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(289); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(290); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(291); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(292); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(293); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(294); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(295); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(296); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(297); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(298); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(299); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(300); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(301); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(302); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(284); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(303); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(304); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(305); break;
  }
 }

 void state287() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__subterm_999_comma_subterm_1000(Base::args(3)));
    break;
  }
 }

 void state288() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_atom__name(Base::args(1)));
    break;
  }
 }

 void state289() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__atom(Base::args(1)));
    break;
  }
 }

 void state290() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_constant__number(Base::args(1)));
    break;
  }
 }

 void state291() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(29); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(30); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(31); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_LISTEXPR: Base::shift_and_goto_state(306); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(307); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(35); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(36); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(37); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(38); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(39); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(41); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(42); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(43); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(44); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(45); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(46); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(47); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(48); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(49); break;
  }
 }

 void state292() {
  switch (lookahead().ordinal()) {
   case SYMBOL_INF: Base::shift_and_goto_state(309); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(310); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(311); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(304); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(305); break;
  }
 }

 void state293() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__unsigned_number(Base::args(1)));
    break;
  }
 }

 void state294() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
  }
 }

 void state295() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
  }
 }

 void state296() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__constant(Base::args(1)));
    break;
  }
 }

 void state297() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(57); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(58); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(59); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(60); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(62); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(63); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(64); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(65); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(67); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(68); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(69); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(70); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(71); break;
   case SYMBOL_ARGUMENTS: Base::shift_and_goto_state(312); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(73); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(74); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(75); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(76); break;
  }
 }

 void state298() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(85); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(86); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(87); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(88); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(314); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(91); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state299() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__list(Base::args(1)));
    break;
  }
 }

 void state300() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(131); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(132); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(133); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(134); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(316); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(136); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(137); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state301() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__string(Base::args(1)));
    break;
  }
 }

 void state302() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__variable(Base::args(1)));
    break;
  }
 }

 void state303() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__subterm_999_bin_op_999_subterm_999(Base::args(3)));
    break;
  }
 }

 void state304() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
  }
 }

 void state305() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
  }
 }

 void state306() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(308); break;
  }
 }

 void state307() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
  }
 }

 void state308() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
  }
 }

 void state309() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_inf(Base::args(2)));
    break;
  }
 }

 void state310() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_nan(Base::args(2)));
    break;
  }
 }

 void state311() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_number__sign_unsigned_number(Base::args(2)));
    break;
  }
 }

 void state312() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(313); break;
  }
 }

 void state313() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
  }
 }

 void state314() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(315); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(129); break;
  }
 }

 void state315() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
  }
 }

 void state316() {
  switch (lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(317); break;
   case SYMBOL_BIN_OP_1200: Base::shift_and_goto_state(177); break;
  }
 }

 void state317() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
  }
 }

 void state318() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__unary_op_999_subterm_999(Base::args(2)));
    break;
  }
 }

 void state319() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1000__unary_op_1000_subterm_1000(Base::args(2)));
    break;
  }
 }

 void state320() {
  switch (lookahead().ordinal()) {
    Base::push(Base::lookahead().ordinal(), Base::reduce_start__subterm_1200_full_stop(Base::args(2)));
    break;
  }
 }

 void state321() {
  switch (lookahead().ordinal()) {
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_ATOM: Base::shift_and_goto_state(2); break;
   case SYMBOL_NUMBER: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAME_LPAREN: Base::shift_and_goto_state(4); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(5); break;
   case SYMBOL_SIGN: Base::shift_and_goto_state(6); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::shift_and_goto_state(7); break;
   case SYMBOL_MINUS: Base::shift_and_goto_state(8); break;
   case SYMBOL_PLUS: Base::shift_and_goto_state(9); break;
   case SYMBOL_TERM_1000: Base::shift_and_goto_state(10); break;
   case SYMBOL_TERM_1200: Base::shift_and_goto_state(11); break;
   case SYMBOL_TERM_0: Base::shift_and_goto_state(12); break;
   case SYMBOL_TERM_999: Base::shift_and_goto_state(13); break;
   case SYMBOL_CONSTANT: Base::shift_and_goto_state(14); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(15); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(16); break;
   case SYMBOL_LIST: Base::shift_and_goto_state(17); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(18); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(19); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(20); break;
   case SYMBOL_SUBTERM_1000: Base::shift_and_goto_state(21); break;
   case SYMBOL_UNARY_OP_1000: Base::shift_and_goto_state(22); break;
   case SYMBOL_UNARY_OP_1200: Base::shift_and_goto_state(24); break;
   case SYMBOL_SUBTERM_1200: Base::shift_and_goto_state(322); break;
   case SYMBOL_SUBTERM_999: Base::shift_and_goto_state(25); break;
   case SYMBOL_UNARY_OP_999: Base::shift_and_goto_state(26); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(27); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(28); break;
  }
 }

 void state322() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1200__subterm_1200_bin_op_1200_subterm_1200(Base::args(3)));
    break;
  }
 }

 void state323() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1200:
   case SYMBOL_FULL_STOP:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_1200__unary_op_1200_subterm_1200(Base::args(2)));
    break;
  }
 }

 void state324() {
  switch (lookahead().ordinal()) {
   case SYMBOL_BIN_OP_1000:
   case SYMBOL_BIN_OP_999:
   case SYMBOL_COMMA:
    Base::push(Base::lookahead().ordinal(), Base::reduce_term_999__unary_op_999_subterm_999(Base::args(2)));
    break;
  }
 }

};

} } 
