namespace prologcoin { namespace common { 

 enum symbol_t {
  SYMBOL_UNKNOWN = 0,
  SYMBOL_EOF = 1000,
  SYMBOL_ARGUMENTS = 1,
  SYMBOL_ATOM = 2,
  SYMBOL_CONSTANT = 3,
  SYMBOL_LIST = 4,
  SYMBOL_LISTEXPR = 5,
  SYMBOL_NUMBER = 6,
  SYMBOL_START = 7,
  SYMBOL_SUBTERM_1000 = 8,
  SYMBOL_SUBTERM_1200 = 9,
  SYMBOL_SUBTERM_999 = 10,
  SYMBOL_TERM_0 = 11,
  SYMBOL_TERM_1000 = 12,
  SYMBOL_TERM_1200 = 13,
  SYMBOL_TERM_999 = 14,
  SYMBOL_UNSIGNED_NUMBER = 15,
  SYMBOL_COMMA = 1001,
  SYMBOL_EMPTY = 1002,
  SYMBOL_FULL_STOP = 1003,
  SYMBOL_FUNCTOR_LPAREN = 1004,
  SYMBOL_INF = 1005,
  SYMBOL_LBRACE = 1006,
  SYMBOL_LBRACKET = 1007,
  SYMBOL_LPAREN = 1008,
  SYMBOL_NAME = 1009,
  SYMBOL_NAN = 1010,
  SYMBOL_NATURAL_NUMBER = 1011,
  SYMBOL_OP_1000 = 1012,
  SYMBOL_OP_1200 = 1013,
  SYMBOL_OP_999 = 1014,
  SYMBOL_RBRACE = 1015,
  SYMBOL_RBRACKET = 1016,
  SYMBOL_RPAREN = 1017,
  SYMBOL_STRING = 1018,
  SYMBOL_UNSIGNED_FLOAT = 1019,
  SYMBOL_VARIABLE = 1020,
  SYMBOL_VBAR = 1021
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
  } }

 void state0() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(5); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(4); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(6); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(25); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(21); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(23); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(26); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(2); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(3); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(20); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(22); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(24); break;
   case SYMBOL_TERM_0: Base::goto_state(10); break;
   case SYMBOL_TERM_1000: Base::goto_state(8); break;
   case SYMBOL_TERM_1200: Base::goto_state(9); break;
   case SYMBOL_TERM_999: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(7); break;
   default: Base::parse_error(); break;
  }
 }

 void state1() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state2() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state3() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state4() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(31); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(46); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state5() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state6() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state7() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state8() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_1000(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state9() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_1200(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state10() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_0(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state11() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_999(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state12() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state13() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(231); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(71); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state14() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(99); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(98); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(233); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(88); break;
   case SYMBOL_TERM_1000: Base::goto_state(86); break;
   case SYMBOL_TERM_1200: Base::goto_state(87); break;
   case SYMBOL_TERM_999: Base::goto_state(89); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state15() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state16() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(134); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(133); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(235); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(123); break;
   case SYMBOL_TERM_1000: Base::goto_state(121); break;
   case SYMBOL_TERM_1200: Base::goto_state(122); break;
   case SYMBOL_TERM_999: Base::goto_state(124); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state17() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state18() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state19() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(5); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(4); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(6); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(25); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(23); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(26); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(2); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(3); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(240); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(24); break;
   case SYMBOL_TERM_0: Base::goto_state(238); break;
   case SYMBOL_TERM_1000: Base::goto_state(237); break;
   case SYMBOL_TERM_999: Base::goto_state(239); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(7); break;
   default: Base::parse_error(); break;
  }
 }

 void state20() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000: Base::shift_and_goto_state(241); break;
   default: Base::parse_error(); break;
  }
 }

 void state21() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(5); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(4); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(6); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(25); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(21); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(23); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(26); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(2); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(3); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(20); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(243); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(24); break;
   case SYMBOL_TERM_0: Base::goto_state(10); break;
   case SYMBOL_TERM_1000: Base::goto_state(8); break;
   case SYMBOL_TERM_1200: Base::goto_state(9); break;
   case SYMBOL_TERM_999: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(7); break;
   default: Base::parse_error(); break;
  }
 }

 void state22() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP: Base::shift_and_goto_state(246); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(244); break;
   default: Base::parse_error(); break;
  }
 }

 void state23() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(5); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(4); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(6); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(25); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(23); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(26); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(2); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(3); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(249); break;
   case SYMBOL_TERM_0: Base::goto_state(247); break;
   case SYMBOL_TERM_999: Base::goto_state(248); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(7); break;
   default: Base::parse_error(); break;
  }
 }

 void state24() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(252); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(250); break;
   default: Base::parse_error(); break;
  }
 }

 void state25() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state26() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state27() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state28() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state29() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state30() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(50); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(49); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(46); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state31() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(52); break;
   default: Base::parse_error(); break;
  }
 }

 void state32() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state33() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state34() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state35() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state36() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state37() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state38() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state39() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(69); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(71); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state40() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(99); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(98); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(189); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(88); break;
   case SYMBOL_TERM_1000: Base::goto_state(86); break;
   case SYMBOL_TERM_1200: Base::goto_state(87); break;
   case SYMBOL_TERM_999: Base::goto_state(89); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state41() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state42() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(134); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(133); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(191); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(123); break;
   case SYMBOL_TERM_1000: Base::goto_state(121); break;
   case SYMBOL_TERM_1200: Base::goto_state(122); break;
   case SYMBOL_TERM_999: Base::goto_state(124); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state43() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state44() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state45() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(193); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state46() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(196); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(194); break;
   case SYMBOL_VBAR: Base::shift_and_goto_state(197); break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LISTEXPR, Base::reduce_listexpr__subterm_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state47() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state48() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state49() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(51); break;
   default: Base::parse_error(); break;
  }
 }

 void state50() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state51() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state52() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state53() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state54() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state55() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state56() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(75); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(74); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(46); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state57() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state58() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state59() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state60() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state61() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state62() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state63() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(77); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(71); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state64() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(99); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(98); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(100); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(88); break;
   case SYMBOL_TERM_1000: Base::goto_state(86); break;
   case SYMBOL_TERM_1200: Base::goto_state(87); break;
   case SYMBOL_TERM_999: Base::goto_state(89); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state65() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state66() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(134); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(133); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(181); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(123); break;
   case SYMBOL_TERM_1000: Base::goto_state(121); break;
   case SYMBOL_TERM_1200: Base::goto_state(122); break;
   case SYMBOL_TERM_999: Base::goto_state(124); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state67() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state68() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state69() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(183); break;
   default: Base::parse_error(); break;
  }
 }

 void state70() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(184); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state71() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(187); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(185); break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ARGUMENTS, Base::reduce_arguments__subterm_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state72() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state73() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state74() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(76); break;
   default: Base::parse_error(); break;
  }
 }

 void state75() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state76() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state77() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(78); break;
   default: Base::parse_error(); break;
  }
 }

 void state78() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state79() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state80() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state81() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state82() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(106); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(105); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(46); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state83() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state84() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state85() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state86() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_1000(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state87() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_1200(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state88() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_0(Base::args(1)));
    break;
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state89() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state90() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state91() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(108); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(71); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state92() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(99); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(98); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(110); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(88); break;
   case SYMBOL_TERM_1000: Base::goto_state(86); break;
   case SYMBOL_TERM_1200: Base::goto_state(87); break;
   case SYMBOL_TERM_999: Base::goto_state(89); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state93() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state94() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(134); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(133); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(135); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(123); break;
   case SYMBOL_TERM_1000: Base::goto_state(121); break;
   case SYMBOL_TERM_1200: Base::goto_state(122); break;
   case SYMBOL_TERM_999: Base::goto_state(124); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state95() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state96() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state97() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(169); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(167); break;
   case SYMBOL_TERM_1000: Base::goto_state(166); break;
   case SYMBOL_TERM_999: Base::goto_state(168); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state98() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000: Base::shift_and_goto_state(170); break;
   default: Base::parse_error(); break;
  }
 }

 void state99() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(99); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(98); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(172); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(88); break;
   case SYMBOL_TERM_1000: Base::goto_state(86); break;
   case SYMBOL_TERM_1200: Base::goto_state(87); break;
   case SYMBOL_TERM_999: Base::goto_state(89); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state100() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(112); break;
   case SYMBOL_RBRACE: Base::shift_and_goto_state(173); break;
   default: Base::parse_error(); break;
  }
 }

 void state101() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(176); break;
   case SYMBOL_TERM_0: Base::goto_state(174); break;
   case SYMBOL_TERM_999: Base::goto_state(175); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state102() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(179); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(177); break;
   default: Base::parse_error(); break;
  }
 }

 void state103() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state104() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state105() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(107); break;
   default: Base::parse_error(); break;
  }
 }

 void state106() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state107() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state108() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(109); break;
   default: Base::parse_error(); break;
  }
 }

 void state109() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state110() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(112); break;
   case SYMBOL_RBRACE: Base::shift_and_goto_state(111); break;
   default: Base::parse_error(); break;
  }
 }

 void state111() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state112() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(99); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(98); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(113); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(88); break;
   case SYMBOL_TERM_1000: Base::goto_state(86); break;
   case SYMBOL_TERM_1200: Base::goto_state(87); break;
   case SYMBOL_TERM_999: Base::goto_state(89); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state113() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_1200, Base::reduce_term_1200__subterm_1200_op_1200_subterm_1200(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state114() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state115() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state116() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state117() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(141); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(140); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(46); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state118() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state119() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state120() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state121() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_1000(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state122() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_1200(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state123() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_0(Base::args(1)));
    break;
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state124() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state125() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state126() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(143); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(71); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state127() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(99); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(98); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(145); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(88); break;
   case SYMBOL_TERM_1000: Base::goto_state(86); break;
   case SYMBOL_TERM_1200: Base::goto_state(87); break;
   case SYMBOL_TERM_999: Base::goto_state(89); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state128() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state129() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(134); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(133); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(147); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(123); break;
   case SYMBOL_TERM_1000: Base::goto_state(121); break;
   case SYMBOL_TERM_1200: Base::goto_state(122); break;
   case SYMBOL_TERM_999: Base::goto_state(124); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state130() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state131() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state132() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(154); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(152); break;
   case SYMBOL_TERM_1000: Base::goto_state(151); break;
   case SYMBOL_TERM_999: Base::goto_state(153); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state133() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000: Base::shift_and_goto_state(155); break;
   default: Base::parse_error(); break;
  }
 }

 void state134() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(134); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(133); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(157); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(123); break;
   case SYMBOL_TERM_1000: Base::goto_state(121); break;
   case SYMBOL_TERM_1200: Base::goto_state(122); break;
   case SYMBOL_TERM_999: Base::goto_state(124); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state135() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(149); break;
   case SYMBOL_RPAREN: Base::shift_and_goto_state(158); break;
   default: Base::parse_error(); break;
  }
 }

 void state136() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(161); break;
   case SYMBOL_TERM_0: Base::goto_state(159); break;
   case SYMBOL_TERM_999: Base::goto_state(160); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state137() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(164); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(162); break;
   default: Base::parse_error(); break;
  }
 }

 void state138() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state139() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state140() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(142); break;
   default: Base::parse_error(); break;
  }
 }

 void state141() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state142() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state143() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(144); break;
   default: Base::parse_error(); break;
  }
 }

 void state144() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state145() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(112); break;
   case SYMBOL_RBRACE: Base::shift_and_goto_state(146); break;
   default: Base::parse_error(); break;
  }
 }

 void state146() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state147() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(149); break;
   case SYMBOL_RPAREN: Base::shift_and_goto_state(148); break;
   default: Base::parse_error(); break;
  }
 }

 void state148() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state149() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(134); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(133); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(150); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(123); break;
   case SYMBOL_TERM_1000: Base::goto_state(121); break;
   case SYMBOL_TERM_1200: Base::goto_state(122); break;
   case SYMBOL_TERM_999: Base::goto_state(124); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state150() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_1200, Base::reduce_term_1200__subterm_1200_op_1200_subterm_1200(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state151() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state152() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state153() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state154() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__op_1000_subterm_1000(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state155() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(156); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(152); break;
   case SYMBOL_TERM_1000: Base::goto_state(151); break;
   case SYMBOL_TERM_999: Base::goto_state(153); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state156() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__subterm_1000_op_1000_subterm_1000(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state157() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_1200, Base::reduce_term_1200__op_1200_subterm_1200(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state158() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state159() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state160() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state161() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__op_999_subterm_999(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state162() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(163); break;
   case SYMBOL_TERM_0: Base::goto_state(159); break;
   case SYMBOL_TERM_999: Base::goto_state(160); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state163() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__subterm_999_op_999_subterm_999(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state164() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(165); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(152); break;
   case SYMBOL_TERM_1000: Base::goto_state(151); break;
   case SYMBOL_TERM_999: Base::goto_state(153); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state165() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__subterm_999_comma_subterm_1000(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state166() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state167() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state168() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state169() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__op_1000_subterm_1000(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state170() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(171); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(167); break;
   case SYMBOL_TERM_1000: Base::goto_state(166); break;
   case SYMBOL_TERM_999: Base::goto_state(168); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state171() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__subterm_1000_op_1000_subterm_1000(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state172() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_1200, Base::reduce_term_1200__op_1200_subterm_1200(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state173() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state174() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state175() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state176() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__op_999_subterm_999(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state177() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(178); break;
   case SYMBOL_TERM_0: Base::goto_state(174); break;
   case SYMBOL_TERM_999: Base::goto_state(175); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state178() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__subterm_999_op_999_subterm_999(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state179() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(180); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(167); break;
   case SYMBOL_TERM_1000: Base::goto_state(166); break;
   case SYMBOL_TERM_999: Base::goto_state(168); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state180() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__subterm_999_comma_subterm_1000(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state181() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(149); break;
   case SYMBOL_RPAREN: Base::shift_and_goto_state(182); break;
   default: Base::parse_error(); break;
  }
 }

 void state182() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state183() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state184() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__op_999_subterm_999(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state185() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(186); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state186() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__subterm_999_op_999_subterm_999(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state187() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(188); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(71); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state188() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ARGUMENTS, Base::reduce_arguments__subterm_999_comma_arguments(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state189() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(112); break;
   case SYMBOL_RBRACE: Base::shift_and_goto_state(190); break;
   default: Base::parse_error(); break;
  }
 }

 void state190() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state191() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(149); break;
   case SYMBOL_RPAREN: Base::shift_and_goto_state(192); break;
   default: Base::parse_error(); break;
  }
 }

 void state192() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state193() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__op_999_subterm_999(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state194() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(195); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state195() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__subterm_999_op_999_subterm_999(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state196() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(198); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(46); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state197() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(209); break;
   case SYMBOL_INF: Base::shift_and_goto_state(203); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(210); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(202); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(212); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(199); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(204); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(217); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(215); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(213); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(218); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(214); break;
   case SYMBOL_ATOM: Base::goto_state(200); break;
   case SYMBOL_CONSTANT: Base::goto_state(208); break;
   case SYMBOL_LIST: Base::goto_state(211); break;
   case SYMBOL_NUMBER: Base::goto_state(201); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(216); break;
   case SYMBOL_TERM_0: Base::goto_state(206); break;
   case SYMBOL_TERM_999: Base::goto_state(207); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(205); break;
   default: Base::parse_error(); break;
  }
 }

 void state198() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LISTEXPR, Base::reduce_listexpr__subterm_999_comma_listexpr(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state199() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state200() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state201() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state202() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(33); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(34); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(47); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(45); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(220); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(48); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(219); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(46); break;
   case SYMBOL_TERM_0: Base::goto_state(36); break;
   case SYMBOL_TERM_999: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(35); break;
   default: Base::parse_error(); break;
  }
 }

 void state203() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state204() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state205() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state206() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state207() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state208() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state209() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(63); break;
   case SYMBOL_INF: Base::shift_and_goto_state(57); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(64); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(56); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(53); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(58); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(72); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(70); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(67); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(73); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(68); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(222); break;
   case SYMBOL_ATOM: Base::goto_state(54); break;
   case SYMBOL_CONSTANT: Base::goto_state(62); break;
   case SYMBOL_LIST: Base::goto_state(65); break;
   case SYMBOL_NUMBER: Base::goto_state(55); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(71); break;
   case SYMBOL_TERM_0: Base::goto_state(60); break;
   case SYMBOL_TERM_999: Base::goto_state(61); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(59); break;
   default: Base::parse_error(); break;
  }
 }

 void state210() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(91); break;
   case SYMBOL_INF: Base::shift_and_goto_state(83); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(82); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(94); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(79); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(84); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(103); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(97); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(99); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(101); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(95); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(104); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(96); break;
   case SYMBOL_ATOM: Base::goto_state(80); break;
   case SYMBOL_CONSTANT: Base::goto_state(90); break;
   case SYMBOL_LIST: Base::goto_state(93); break;
   case SYMBOL_NUMBER: Base::goto_state(81); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(98); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(224); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(102); break;
   case SYMBOL_TERM_0: Base::goto_state(88); break;
   case SYMBOL_TERM_1000: Base::goto_state(86); break;
   case SYMBOL_TERM_1200: Base::goto_state(87); break;
   case SYMBOL_TERM_999: Base::goto_state(89); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(85); break;
   default: Base::parse_error(); break;
  }
 }

 void state211() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state212() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(126); break;
   case SYMBOL_INF: Base::shift_and_goto_state(118); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(127); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(117); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(129); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(114); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(119); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(138); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(132); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(134); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(130); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(139); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(131); break;
   case SYMBOL_ATOM: Base::goto_state(115); break;
   case SYMBOL_CONSTANT: Base::goto_state(125); break;
   case SYMBOL_LIST: Base::goto_state(128); break;
   case SYMBOL_NUMBER: Base::goto_state(116); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(133); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(226); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(123); break;
   case SYMBOL_TERM_1000: Base::goto_state(121); break;
   case SYMBOL_TERM_1200: Base::goto_state(122); break;
   case SYMBOL_TERM_999: Base::goto_state(124); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(120); break;
   default: Base::parse_error(); break;
  }
 }

 void state213() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state214() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state215() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(209); break;
   case SYMBOL_INF: Base::shift_and_goto_state(203); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(210); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(202); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(212); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(199); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(204); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(217); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(215); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(213); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(218); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(214); break;
   case SYMBOL_ATOM: Base::goto_state(200); break;
   case SYMBOL_CONSTANT: Base::goto_state(208); break;
   case SYMBOL_LIST: Base::goto_state(211); break;
   case SYMBOL_NUMBER: Base::goto_state(201); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(228); break;
   case SYMBOL_TERM_0: Base::goto_state(206); break;
   case SYMBOL_TERM_999: Base::goto_state(207); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(205); break;
   default: Base::parse_error(); break;
  }
 }

 void state216() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999: Base::shift_and_goto_state(229); break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LISTEXPR, Base::reduce_listexpr__subterm_999_vbar_subterm_999(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state217() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state218() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state219() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(221); break;
   default: Base::parse_error(); break;
  }
 }

 void state220() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state221() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state222() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(223); break;
   default: Base::parse_error(); break;
  }
 }

 void state223() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state224() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(112); break;
   case SYMBOL_RBRACE: Base::shift_and_goto_state(225); break;
   default: Base::parse_error(); break;
  }
 }

 void state225() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state226() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(149); break;
   case SYMBOL_RPAREN: Base::shift_and_goto_state(227); break;
   default: Base::parse_error(); break;
  }
 }

 void state227() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state228() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__op_999_subterm_999(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state229() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(209); break;
   case SYMBOL_INF: Base::shift_and_goto_state(203); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(210); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(202); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(212); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(199); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(204); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(217); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(215); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(213); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(218); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(214); break;
   case SYMBOL_ATOM: Base::goto_state(200); break;
   case SYMBOL_CONSTANT: Base::goto_state(208); break;
   case SYMBOL_LIST: Base::goto_state(211); break;
   case SYMBOL_NUMBER: Base::goto_state(201); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(230); break;
   case SYMBOL_TERM_0: Base::goto_state(206); break;
   case SYMBOL_TERM_999: Base::goto_state(207); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(205); break;
   default: Base::parse_error(); break;
  }
 }

 void state230() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_999:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__subterm_999_op_999_subterm_999(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state231() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(232); break;
   default: Base::parse_error(); break;
  }
 }

 void state232() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state233() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(112); break;
   case SYMBOL_RBRACE: Base::shift_and_goto_state(234); break;
   default: Base::parse_error(); break;
  }
 }

 void state234() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state235() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_1200: Base::shift_and_goto_state(149); break;
   case SYMBOL_RPAREN: Base::shift_and_goto_state(236); break;
   default: Base::parse_error(); break;
  }
 }

 void state236() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state237() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_1000(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state238() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state239() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_SUBTERM_1000, Base::reduce_subterm_1000__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state240() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__op_1000_subterm_1000(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state241() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(5); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(4); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(6); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(25); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(23); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(26); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(2); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(3); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(242); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(24); break;
   case SYMBOL_TERM_0: Base::goto_state(238); break;
   case SYMBOL_TERM_1000: Base::goto_state(237); break;
   case SYMBOL_TERM_999: Base::goto_state(239); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(7); break;
   default: Base::parse_error(); break;
  }
 }

 void state242() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__subterm_1000_op_1000_subterm_1000(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state243() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_TERM_1200, Base::reduce_term_1200__op_1200_subterm_1200(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state244() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(5); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(4); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(6); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(25); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_1200: Base::shift_and_goto_state(21); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(23); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(26); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(2); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(3); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(20); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(245); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(24); break;
   case SYMBOL_TERM_0: Base::goto_state(10); break;
   case SYMBOL_TERM_1000: Base::goto_state(8); break;
   case SYMBOL_TERM_1200: Base::goto_state(9); break;
   case SYMBOL_TERM_999: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(7); break;
   default: Base::parse_error(); break;
  }
 }

 void state245() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_TERM_1200, Base::reduce_term_1200__subterm_1200_op_1200_subterm_1200(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state246() {
  switch (Base::lookahead().ordinal()) {
   default: 
    Base::reduce(SYMBOL_START, Base::reduce_start__subterm_1200_full_stop(Base::args(2)));
    break;
  }
 }

 void state247() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_0(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state248() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__term_999(Base::args(1)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state249() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__op_999_subterm_999(Base::args(2)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state250() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(5); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(4); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(6); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(25); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(23); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(26); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(2); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(3); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(251); break;
   case SYMBOL_TERM_0: Base::goto_state(247); break;
   case SYMBOL_TERM_999: Base::goto_state(248); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(7); break;
   default: Base::parse_error(); break;
  }
 }

 void state251() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
   case SYMBOL_OP_999:
    Base::reduce(SYMBOL_TERM_999, Base::reduce_term_999__subterm_999_op_999_subterm_999(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

 void state252() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(5); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(4); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(1); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(6); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(25); break;
   case SYMBOL_OP_1000: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_999: Base::shift_and_goto_state(23); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(26); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(2); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(3); break;
   case SYMBOL_SUBTERM_1000: Base::goto_state(253); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(24); break;
   case SYMBOL_TERM_0: Base::goto_state(238); break;
   case SYMBOL_TERM_1000: Base::goto_state(237); break;
   case SYMBOL_TERM_999: Base::goto_state(239); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(7); break;
   default: Base::parse_error(); break;
  }
 }

 void state253() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_1000:
   case SYMBOL_OP_1200:
    Base::reduce(SYMBOL_TERM_1000, Base::reduce_term_1000__subterm_999_comma_subterm_1000(Base::args(3)));
    break;
   default: Base::parse_error(); break;
  }
 }

};

} } 
