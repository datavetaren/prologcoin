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
  SYMBOL_SUBTERM_1200 = 8,
  SYMBOL_SUBTERM_999 = 9,
  SYMBOL_SUBTERM_N = 10,
  SYMBOL_TERM_0 = 11,
  SYMBOL_TERM_N = 12,
  SYMBOL_UNSIGNED_NUMBER = 13,
  SYMBOL_COMMA = 1001,
  SYMBOL_EMPTY = 1002,
  SYMBOL_EMPTY_BRACE = 1003,
  SYMBOL_EMPTY_LIST = 1004,
  SYMBOL_FULL_STOP = 1005,
  SYMBOL_FUNCTOR_LPAREN = 1006,
  SYMBOL_INF = 1007,
  SYMBOL_LBRACE = 1008,
  SYMBOL_LBRACKET = 1009,
  SYMBOL_LPAREN = 1010,
  SYMBOL_NAME = 1011,
  SYMBOL_NAN = 1012,
  SYMBOL_NATURAL_NUMBER = 1013,
  SYMBOL_OP_FX = 1014,
  SYMBOL_OP_FY = 1015,
  SYMBOL_OP_XF = 1016,
  SYMBOL_OP_XFX = 1017,
  SYMBOL_OP_XFY = 1018,
  SYMBOL_OP_YF = 1019,
  SYMBOL_OP_YFX = 1020,
  SYMBOL_RBRACE = 1021,
  SYMBOL_RBRACKET = 1022,
  SYMBOL_RPAREN = 1023,
  SYMBOL_STRING = 1024,
  SYMBOL_UNSIGNED_FLOAT = 1025,
  SYMBOL_VARIABLE = 1026,
  SYMBOL_VBAR = 1027
 };

template<typename Base, typename... Args> class term_parser_gen : public Base {
 protected:

 term_parser_gen(Args&... args) : Base(args...) { }

 std::string symbol_name(symbol_t sym) const {
  switch (sym) {
   case SYMBOL_ARGUMENTS: return "arguments";
   case SYMBOL_ATOM: return "atom";
   case SYMBOL_CONSTANT: return "constant";
   case SYMBOL_LIST: return "list";
   case SYMBOL_LISTEXPR: return "listexpr";
   case SYMBOL_NUMBER: return "number";
   case SYMBOL_START: return "start";
   case SYMBOL_SUBTERM_1200: return "subterm_1200";
   case SYMBOL_SUBTERM_999: return "subterm_999";
   case SYMBOL_SUBTERM_N: return "subterm_n";
   case SYMBOL_TERM_0: return "term_0";
   case SYMBOL_TERM_N: return "term_n";
   case SYMBOL_UNSIGNED_NUMBER: return "unsigned_number";
   case SYMBOL_COMMA: return "comma";
   case SYMBOL_EMPTY: return "empty";
   case SYMBOL_EMPTY_BRACE: return "empty_brace";
   case SYMBOL_EMPTY_LIST: return "empty_list";
   case SYMBOL_FULL_STOP: return "full_stop";
   case SYMBOL_FUNCTOR_LPAREN: return "functor_lparen";
   case SYMBOL_INF: return "inf";
   case SYMBOL_LBRACE: return "lbrace";
   case SYMBOL_LBRACKET: return "lbracket";
   case SYMBOL_LPAREN: return "lparen";
   case SYMBOL_NAME: return "name";
   case SYMBOL_NAN: return "nan";
   case SYMBOL_NATURAL_NUMBER: return "natural_number";
   case SYMBOL_OP_FX: return "op_fx";
   case SYMBOL_OP_FY: return "op_fy";
   case SYMBOL_OP_XF: return "op_xf";
   case SYMBOL_OP_XFX: return "op_xfx";
   case SYMBOL_OP_XFY: return "op_xfy";
   case SYMBOL_OP_YF: return "op_yf";
   case SYMBOL_OP_YFX: return "op_yfx";
   case SYMBOL_RBRACE: return "rbrace";
   case SYMBOL_RBRACKET: return "rbracket";
   case SYMBOL_RPAREN: return "rparen";
   case SYMBOL_STRING: return "string";
   case SYMBOL_UNSIGNED_FLOAT: return "unsigned_float";
   case SYMBOL_VARIABLE: return "variable";
   case SYMBOL_VBAR: return "vbar";
   default: return "?";
  }
 }

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
  }
 }

 std::vector<std::string> state_description() const {
  switch (Base::current_state()) {
   case 0: return state_strings_0();
   case 1: return state_strings_1();
   case 2: return state_strings_2();
   case 3: return state_strings_3();
   case 4: return state_strings_4();
   case 5: return state_strings_5();
   case 6: return state_strings_6();
   case 7: return state_strings_7();
   case 8: return state_strings_8();
   case 9: return state_strings_9();
   case 10: return state_strings_10();
   case 11: return state_strings_11();
   case 12: return state_strings_12();
   case 13: return state_strings_13();
   case 14: return state_strings_14();
   case 15: return state_strings_15();
   case 16: return state_strings_16();
   case 17: return state_strings_17();
   case 18: return state_strings_18();
   case 19: return state_strings_19();
   case 20: return state_strings_20();
   case 21: return state_strings_21();
   case 22: return state_strings_22();
   case 23: return state_strings_23();
   case 24: return state_strings_24();
   case 25: return state_strings_25();
   case 26: return state_strings_26();
   case 27: return state_strings_27();
   case 28: return state_strings_28();
   case 29: return state_strings_29();
   case 30: return state_strings_30();
   case 31: return state_strings_31();
   case 32: return state_strings_32();
   case 33: return state_strings_33();
   case 34: return state_strings_34();
   case 35: return state_strings_35();
   case 36: return state_strings_36();
   case 37: return state_strings_37();
   case 38: return state_strings_38();
   case 39: return state_strings_39();
   case 40: return state_strings_40();
   case 41: return state_strings_41();
   case 42: return state_strings_42();
   case 43: return state_strings_43();
   case 44: return state_strings_44();
   case 45: return state_strings_45();
   case 46: return state_strings_46();
   case 47: return state_strings_47();
   case 48: return state_strings_48();
   case 49: return state_strings_49();
   case 50: return state_strings_50();
   case 51: return state_strings_51();
   case 52: return state_strings_52();
   case 53: return state_strings_53();
   case 54: return state_strings_54();
   case 55: return state_strings_55();
   case 56: return state_strings_56();
   case 57: return state_strings_57();
   case 58: return state_strings_58();
   case 59: return state_strings_59();
   case 60: return state_strings_60();
   case 61: return state_strings_61();
   case 62: return state_strings_62();
   case 63: return state_strings_63();
   case 64: return state_strings_64();
   case 65: return state_strings_65();
   case 66: return state_strings_66();
   case 67: return state_strings_67();
   case 68: return state_strings_68();
   case 69: return state_strings_69();
   case 70: return state_strings_70();
   case 71: return state_strings_71();
   case 72: return state_strings_72();
   case 73: return state_strings_73();
   case 74: return state_strings_74();
   case 75: return state_strings_75();
   case 76: return state_strings_76();
   case 77: return state_strings_77();
   case 78: return state_strings_78();
   case 79: return state_strings_79();
   case 80: return state_strings_80();
   case 81: return state_strings_81();
   case 82: return state_strings_82();
   case 83: return state_strings_83();
   case 84: return state_strings_84();
   case 85: return state_strings_85();
   case 86: return state_strings_86();
   case 87: return state_strings_87();
   case 88: return state_strings_88();
   case 89: return state_strings_89();
   case 90: return state_strings_90();
   case 91: return state_strings_91();
   case 92: return state_strings_92();
   case 93: return state_strings_93();
   case 94: return state_strings_94();
   case 95: return state_strings_95();
   case 96: return state_strings_96();
   case 97: return state_strings_97();
   case 98: return state_strings_98();
   case 99: return state_strings_99();
   case 100: return state_strings_100();
   case 101: return state_strings_101();
   case 102: return state_strings_102();
   case 103: return state_strings_103();
   case 104: return state_strings_104();
   case 105: return state_strings_105();
   case 106: return state_strings_106();
   case 107: return state_strings_107();
   case 108: return state_strings_108();
   case 109: return state_strings_109();
   case 110: return state_strings_110();
   case 111: return state_strings_111();
   case 112: return state_strings_112();
   case 113: return state_strings_113();
   case 114: return state_strings_114();
   case 115: return state_strings_115();
   case 116: return state_strings_116();
   case 117: return state_strings_117();
   case 118: return state_strings_118();
   case 119: return state_strings_119();
   case 120: return state_strings_120();
   case 121: return state_strings_121();
   case 122: return state_strings_122();
   case 123: return state_strings_123();
   case 124: return state_strings_124();
   case 125: return state_strings_125();
   case 126: return state_strings_126();
   case 127: return state_strings_127();
   case 128: return state_strings_128();
   case 129: return state_strings_129();
   case 130: return state_strings_130();
   case 131: return state_strings_131();
   case 132: return state_strings_132();
   case 133: return state_strings_133();
   case 134: return state_strings_134();
   case 135: return state_strings_135();
   case 136: return state_strings_136();
   case 137: return state_strings_137();
   case 138: return state_strings_138();
   case 139: return state_strings_139();
   case 140: return state_strings_140();
   case 141: return state_strings_141();
   case 142: return state_strings_142();
   case 143: return state_strings_143();
   case 144: return state_strings_144();
   case 145: return state_strings_145();
   case 146: return state_strings_146();
   case 147: return state_strings_147();
   case 148: return state_strings_148();
   case 149: return state_strings_149();
   case 150: return state_strings_150();
   case 151: return state_strings_151();
   case 152: return state_strings_152();
   case 153: return state_strings_153();
   case 154: return state_strings_154();
   case 155: return state_strings_155();
   case 156: return state_strings_156();
   case 157: return state_strings_157();
   case 158: return state_strings_158();
   case 159: return state_strings_159();
   case 160: return state_strings_160();
   case 161: return state_strings_161();
   case 162: return state_strings_162();
   case 163: return state_strings_163();
   case 164: return state_strings_164();
   case 165: return state_strings_165();
   case 166: return state_strings_166();
   case 167: return state_strings_167();
   case 168: return state_strings_168();
   case 169: return state_strings_169();
   case 170: return state_strings_170();
   case 171: return state_strings_171();
   case 172: return state_strings_172();
   case 173: return state_strings_173();
   case 174: return state_strings_174();
   case 175: return state_strings_175();
   case 176: return state_strings_176();
   case 177: return state_strings_177();
   case 178: return state_strings_178();
   case 179: return state_strings_179();
   case 180: return state_strings_180();
   case 181: return state_strings_181();
   case 182: return state_strings_182();
   case 183: return state_strings_183();
   case 184: return state_strings_184();
   case 185: return state_strings_185();
   case 186: return state_strings_186();
   case 187: return state_strings_187();
   case 188: return state_strings_188();
   case 189: return state_strings_189();
   case 190: return state_strings_190();
   case 191: return state_strings_191();
   case 192: return state_strings_192();
   case 193: return state_strings_193();
   case 194: return state_strings_194();
   case 195: return state_strings_195();
   case 196: return state_strings_196();
   case 197: return state_strings_197();
   case 198: return state_strings_198();
   case 199: return state_strings_199();
   case 200: return state_strings_200();
   case 201: return state_strings_201();
   case 202: return state_strings_202();
   case 203: return state_strings_203();
   case 204: return state_strings_204();
   case 205: return state_strings_205();
   case 206: return state_strings_206();
   case 207: return state_strings_207();
   case 208: return state_strings_208();
   case 209: return state_strings_209();
   case 210: return state_strings_210();
   case 211: return state_strings_211();
   case 212: return state_strings_212();
   case 213: return state_strings_213();
   case 214: return state_strings_214();
   case 215: return state_strings_215();
   case 216: return state_strings_216();
   case 217: return state_strings_217();
   case 218: return state_strings_218();
   case 219: return state_strings_219();
   case 220: return state_strings_220();
   case 221: return state_strings_221();
   case 222: return state_strings_222();
   case 223: return state_strings_223();
   case 224: return state_strings_224();
   case 225: return state_strings_225();
   case 226: return state_strings_226();
   case 227: return state_strings_227();
   case 228: return state_strings_228();
   case 229: return state_strings_229();
   case 230: return state_strings_230();
   case 231: return state_strings_231();
   case 232: return state_strings_232();
   case 233: return state_strings_233();
   case 234: return state_strings_234();
   case 235: return state_strings_235();
   case 236: return state_strings_236();
   case 237: return state_strings_237();
   case 238: return state_strings_238();
   case 239: return state_strings_239();
   case 240: return state_strings_240();
   case 241: return state_strings_241();
   case 242: return state_strings_242();
   case 243: return state_strings_243();
   case 244: return state_strings_244();
   case 245: return state_strings_245();
   case 246: return state_strings_246();
   case 247: return state_strings_247();
   case 248: return state_strings_248();
   case 249: return state_strings_249();
   case 250: return state_strings_250();
   case 251: return state_strings_251();
   case 252: return state_strings_252();
   case 253: return state_strings_253();
   case 254: return state_strings_254();
   case 255: return state_strings_255();
   case 256: return state_strings_256();
   case 257: return state_strings_257();
   case 258: return state_strings_258();
   case 259: return state_strings_259();
   case 260: return state_strings_260();
   case 261: return state_strings_261();
   case 262: return state_strings_262();

   default: return std::vector<std::string>();  }
 }

 std::vector<int> state_next_symbols() {
  switch (Base::current_state()) {
   case 0: return state_la_0();
   case 1: return state_la_1();
   case 2: return state_la_2();
   case 3: return state_la_3();
   case 4: return state_la_4();
   case 5: return state_la_5();
   case 6: return state_la_6();
   case 7: return state_la_7();
   case 8: return state_la_8();
   case 9: return state_la_9();
   case 10: return state_la_10();
   case 11: return state_la_11();
   case 12: return state_la_12();
   case 13: return state_la_13();
   case 14: return state_la_14();
   case 15: return state_la_15();
   case 16: return state_la_16();
   case 17: return state_la_17();
   case 18: return state_la_18();
   case 19: return state_la_19();
   case 20: return state_la_20();
   case 21: return state_la_21();
   case 22: return state_la_22();
   case 23: return state_la_23();
   case 24: return state_la_24();
   case 25: return state_la_25();
   case 26: return state_la_26();
   case 27: return state_la_27();
   case 28: return state_la_28();
   case 29: return state_la_29();
   case 30: return state_la_30();
   case 31: return state_la_31();
   case 32: return state_la_32();
   case 33: return state_la_33();
   case 34: return state_la_34();
   case 35: return state_la_35();
   case 36: return state_la_36();
   case 37: return state_la_37();
   case 38: return state_la_38();
   case 39: return state_la_39();
   case 40: return state_la_40();
   case 41: return state_la_41();
   case 42: return state_la_42();
   case 43: return state_la_43();
   case 44: return state_la_44();
   case 45: return state_la_45();
   case 46: return state_la_46();
   case 47: return state_la_47();
   case 48: return state_la_48();
   case 49: return state_la_49();
   case 50: return state_la_50();
   case 51: return state_la_51();
   case 52: return state_la_52();
   case 53: return state_la_53();
   case 54: return state_la_54();
   case 55: return state_la_55();
   case 56: return state_la_56();
   case 57: return state_la_57();
   case 58: return state_la_58();
   case 59: return state_la_59();
   case 60: return state_la_60();
   case 61: return state_la_61();
   case 62: return state_la_62();
   case 63: return state_la_63();
   case 64: return state_la_64();
   case 65: return state_la_65();
   case 66: return state_la_66();
   case 67: return state_la_67();
   case 68: return state_la_68();
   case 69: return state_la_69();
   case 70: return state_la_70();
   case 71: return state_la_71();
   case 72: return state_la_72();
   case 73: return state_la_73();
   case 74: return state_la_74();
   case 75: return state_la_75();
   case 76: return state_la_76();
   case 77: return state_la_77();
   case 78: return state_la_78();
   case 79: return state_la_79();
   case 80: return state_la_80();
   case 81: return state_la_81();
   case 82: return state_la_82();
   case 83: return state_la_83();
   case 84: return state_la_84();
   case 85: return state_la_85();
   case 86: return state_la_86();
   case 87: return state_la_87();
   case 88: return state_la_88();
   case 89: return state_la_89();
   case 90: return state_la_90();
   case 91: return state_la_91();
   case 92: return state_la_92();
   case 93: return state_la_93();
   case 94: return state_la_94();
   case 95: return state_la_95();
   case 96: return state_la_96();
   case 97: return state_la_97();
   case 98: return state_la_98();
   case 99: return state_la_99();
   case 100: return state_la_100();
   case 101: return state_la_101();
   case 102: return state_la_102();
   case 103: return state_la_103();
   case 104: return state_la_104();
   case 105: return state_la_105();
   case 106: return state_la_106();
   case 107: return state_la_107();
   case 108: return state_la_108();
   case 109: return state_la_109();
   case 110: return state_la_110();
   case 111: return state_la_111();
   case 112: return state_la_112();
   case 113: return state_la_113();
   case 114: return state_la_114();
   case 115: return state_la_115();
   case 116: return state_la_116();
   case 117: return state_la_117();
   case 118: return state_la_118();
   case 119: return state_la_119();
   case 120: return state_la_120();
   case 121: return state_la_121();
   case 122: return state_la_122();
   case 123: return state_la_123();
   case 124: return state_la_124();
   case 125: return state_la_125();
   case 126: return state_la_126();
   case 127: return state_la_127();
   case 128: return state_la_128();
   case 129: return state_la_129();
   case 130: return state_la_130();
   case 131: return state_la_131();
   case 132: return state_la_132();
   case 133: return state_la_133();
   case 134: return state_la_134();
   case 135: return state_la_135();
   case 136: return state_la_136();
   case 137: return state_la_137();
   case 138: return state_la_138();
   case 139: return state_la_139();
   case 140: return state_la_140();
   case 141: return state_la_141();
   case 142: return state_la_142();
   case 143: return state_la_143();
   case 144: return state_la_144();
   case 145: return state_la_145();
   case 146: return state_la_146();
   case 147: return state_la_147();
   case 148: return state_la_148();
   case 149: return state_la_149();
   case 150: return state_la_150();
   case 151: return state_la_151();
   case 152: return state_la_152();
   case 153: return state_la_153();
   case 154: return state_la_154();
   case 155: return state_la_155();
   case 156: return state_la_156();
   case 157: return state_la_157();
   case 158: return state_la_158();
   case 159: return state_la_159();
   case 160: return state_la_160();
   case 161: return state_la_161();
   case 162: return state_la_162();
   case 163: return state_la_163();
   case 164: return state_la_164();
   case 165: return state_la_165();
   case 166: return state_la_166();
   case 167: return state_la_167();
   case 168: return state_la_168();
   case 169: return state_la_169();
   case 170: return state_la_170();
   case 171: return state_la_171();
   case 172: return state_la_172();
   case 173: return state_la_173();
   case 174: return state_la_174();
   case 175: return state_la_175();
   case 176: return state_la_176();
   case 177: return state_la_177();
   case 178: return state_la_178();
   case 179: return state_la_179();
   case 180: return state_la_180();
   case 181: return state_la_181();
   case 182: return state_la_182();
   case 183: return state_la_183();
   case 184: return state_la_184();
   case 185: return state_la_185();
   case 186: return state_la_186();
   case 187: return state_la_187();
   case 188: return state_la_188();
   case 189: return state_la_189();
   case 190: return state_la_190();
   case 191: return state_la_191();
   case 192: return state_la_192();
   case 193: return state_la_193();
   case 194: return state_la_194();
   case 195: return state_la_195();
   case 196: return state_la_196();
   case 197: return state_la_197();
   case 198: return state_la_198();
   case 199: return state_la_199();
   case 200: return state_la_200();
   case 201: return state_la_201();
   case 202: return state_la_202();
   case 203: return state_la_203();
   case 204: return state_la_204();
   case 205: return state_la_205();
   case 206: return state_la_206();
   case 207: return state_la_207();
   case 208: return state_la_208();
   case 209: return state_la_209();
   case 210: return state_la_210();
   case 211: return state_la_211();
   case 212: return state_la_212();
   case 213: return state_la_213();
   case 214: return state_la_214();
   case 215: return state_la_215();
   case 216: return state_la_216();
   case 217: return state_la_217();
   case 218: return state_la_218();
   case 219: return state_la_219();
   case 220: return state_la_220();
   case 221: return state_la_221();
   case 222: return state_la_222();
   case 223: return state_la_223();
   case 224: return state_la_224();
   case 225: return state_la_225();
   case 226: return state_la_226();
   case 227: return state_la_227();
   case 228: return state_la_228();
   case 229: return state_la_229();
   case 230: return state_la_230();
   case 231: return state_la_231();
   case 232: return state_la_232();
   case 233: return state_la_233();
   case 234: return state_la_234();
   case 235: return state_la_235();
   case 236: return state_la_236();
   case 237: return state_la_237();
   case 238: return state_la_238();
   case 239: return state_la_239();
   case 240: return state_la_240();
   case 241: return state_la_241();
   case 242: return state_la_242();
   case 243: return state_la_243();
   case 244: return state_la_244();
   case 245: return state_la_245();
   case 246: return state_la_246();
   case 247: return state_la_247();
   case 248: return state_la_248();
   case 249: return state_la_249();
   case 250: return state_la_250();
   case 251: return state_la_251();
   case 252: return state_la_252();
   case 253: return state_la_253();
   case 254: return state_la_254();
   case 255: return state_la_255();
   case 256: return state_la_256();
   case 257: return state_la_257();
   case 258: return state_la_258();
   case 259: return state_la_259();
   case 260: return state_la_260();
   case 261: return state_la_261();
   case 262: return state_la_262();

   default: return std::vector<int>();
  }
 }

 void state0() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(1); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(2); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(7); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(6); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(8); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(23); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(20); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(24); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(4); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(5); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(10); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(21); break;
   case SYMBOL_TERM_0: Base::goto_state(22); break;
   case SYMBOL_TERM_N: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(9); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_0() const {
  std::vector<std::string> s{
   "start :-  [*]  subterm_1200  full_stop "
   };
  return s;
 }

 std::vector<int> state_la_0() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state1() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_brace(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_1() const {
  std::vector<std::string> s{
   "atom :-  empty_brace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_1() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state2() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_2() const {
  std::vector<std::string> s{
   "atom :-  empty_list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_2() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state3() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_3() const {
  std::vector<std::string> s{
   "atom :-  name  [*] "
   };
  return s;
 }

 std::vector<int> state_la_3() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state4() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_4() const {
  std::vector<std::string> s{
   "constant :-  atom  [*] "
   };
  return s;
 }

 std::vector<int> state_la_4() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state5() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_5() const {
  std::vector<std::string> s{
   "constant :-  number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_5() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state6() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(32); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(31); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(33); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(47); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_6() const {
  std::vector<std::string> s{
   "list :-  lbracket  [*]  rbracket ", 
   "      |                 listexpr  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_6() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LISTEXPR, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_RBRACKET, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state7() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_7() const {
  std::vector<std::string> s{
   "number :-  inf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_7() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state8() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_8() const {
  std::vector<std::string> s{
   "number :-  nan  [*] "
   };
  return s;
 }

 std::vector<int> state_la_8() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state9() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_9() const {
  std::vector<std::string> s{
   "number :-  unsigned_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_9() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state10() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP: Base::shift_and_goto_state(246); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_10() const {
  std::vector<std::string> s{
   "start :-  subterm_1200  [*]  full_stop "
   };
  return s;
 }

 std::vector<int> state_la_10() const {
  std::vector<int> s{SYMBOL_FULL_STOP};
  return s;
 }

 void state11() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_SUBTERM_N, Base::reduce_subterm_n__term_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_11() const {
  std::vector<std::string> s{
   "subterm_n :-  term_n  [*] "
   };
  return s;
 }

 std::vector<int> state_la_11() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state12() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_12() const {
  std::vector<std::string> s{
   "term_0 :-  constant  [*] "
   };
  return s;
 }

 std::vector<int> state_la_12() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state13() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(247); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(85); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(106); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_13() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  [*]  arguments  rparen "
   };
  return s;
 }

 std::vector<int> state_la_13() const {
  std::vector<int> s{SYMBOL_ARGUMENTS, SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state14() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(249); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_14() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  [*]  subterm_1200  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_14() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state15() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_15() const {
  std::vector<std::string> s{
   "term_0 :-  list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_15() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state16() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(251); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(168); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_16() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  [*]  subterm_1200  rparen "
   };
  return s;
 }

 std::vector<int> state_la_16() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state17() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_17() const {
  std::vector<std::string> s{
   "term_0 :-  string  [*] "
   };
  return s;
 }

 std::vector<int> state_la_17() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state18() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_18() const {
  std::vector<std::string> s{
   "term_0 :-  variable  [*] "
   };
  return s;
 }

 std::vector<int> state_la_18() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state19() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(1); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(2); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(7); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(6); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(8); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(23); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(20); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(24); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(4); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(5); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(253); break;
   case SYMBOL_TERM_0: Base::goto_state(22); break;
   case SYMBOL_TERM_N: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(9); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_19() const {
  std::vector<std::string> s{
   "term_n :-  op_fx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_19() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state20() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(1); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(2); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(7); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(6); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(8); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(23); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(20); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(24); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(4); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(5); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(262); break;
   case SYMBOL_TERM_0: Base::goto_state(22); break;
   case SYMBOL_TERM_N: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(9); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_20() const {
  std::vector<std::string> s{
   "term_n :-  op_fy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_20() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state21() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF: Base::shift_and_goto_state(254); break;
   case SYMBOL_OP_XFX: Base::shift_and_goto_state(255); break;
   case SYMBOL_OP_XFY: Base::shift_and_goto_state(256); break;
   case SYMBOL_OP_YF: Base::shift_and_goto_state(257); break;
   case SYMBOL_OP_YFX: Base::shift_and_goto_state(258); break;
   case SYMBOL_FULL_STOP:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__subterm_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_21() const {
  std::vector<std::string> s{
   "subterm_1200 :-  subterm_n  [*] ", 
   "term_n       :-                  op_xf  ", 
   "              |                  op_yf  ", 
   "              |                  op_xfx  subterm_n ", 
   "              |                  op_xfy            ", 
   "              |                  op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_21() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state22() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__term_0(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_22() const {
  std::vector<std::string> s{
   "term_n :-  term_0  [*] "
   };
  return s;
 }

 std::vector<int> state_la_22() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state23() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_23() const {
  std::vector<std::string> s{
   "unsigned_number :-  natural_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_23() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state24() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_24() const {
  std::vector<std::string> s{
   "unsigned_number :-  unsigned_float  [*] "
   };
  return s;
 }

 std::vector<int> state_la_24() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state25() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_brace(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_25() const {
  std::vector<std::string> s{
   "atom :-  empty_brace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_25() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state26() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_26() const {
  std::vector<std::string> s{
   "atom :-  empty_list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_26() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state27() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_27() const {
  std::vector<std::string> s{
   "atom :-  name  [*] "
   };
  return s;
 }

 std::vector<int> state_la_27() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state28() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_28() const {
  std::vector<std::string> s{
   "constant :-  atom  [*] "
   };
  return s;
 }

 std::vector<int> state_la_28() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state29() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_29() const {
  std::vector<std::string> s{
   "constant :-  number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_29() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state30() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(52); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(51); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(33); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(47); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_30() const {
  std::vector<std::string> s{
   "list :-  lbracket  [*]  rbracket ", 
   "      |                 listexpr  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_30() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LISTEXPR, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_RBRACKET, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state31() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(54); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_31() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  [*]  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_31() const {
  std::vector<int> s{SYMBOL_RBRACKET};
  return s;
 }

 void state32() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_32() const {
  std::vector<std::string> s{
   "list :-  lbracket  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_32() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state33() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(55); break;
   case SYMBOL_VBAR: Base::shift_and_goto_state(56); break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LISTEXPR, Base::reduce_listexpr__subterm_999(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_33() const {
  std::vector<std::string> s{
   "listexpr :-  subterm_999  [*] ", 
   "          |                    comma  listexpr    ", 
   "          |                    vbar   subterm_999 "
   };
  return s;
 }

 std::vector<int> state_la_33() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state34() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_34() const {
  std::vector<std::string> s{
   "number :-  inf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_34() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state35() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_35() const {
  std::vector<std::string> s{
   "number :-  nan  [*] "
   };
  return s;
 }

 std::vector<int> state_la_35() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state36() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_36() const {
  std::vector<std::string> s{
   "number :-  unsigned_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_36() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state37() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_SUBTERM_N, Base::reduce_subterm_n__term_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_37() const {
  std::vector<std::string> s{
   "subterm_n :-  term_n  [*] "
   };
  return s;
 }

 std::vector<int> state_la_37() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state38() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_38() const {
  std::vector<std::string> s{
   "term_0 :-  constant  [*] "
   };
  return s;
 }

 std::vector<int> state_la_38() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state39() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(230); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(85); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(106); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_39() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  [*]  arguments  rparen "
   };
  return s;
 }

 std::vector<int> state_la_39() const {
  std::vector<int> s{SYMBOL_ARGUMENTS, SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state40() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(232); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_40() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  [*]  subterm_1200  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_40() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state41() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_41() const {
  std::vector<std::string> s{
   "term_0 :-  list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_41() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state42() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(234); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(168); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_42() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  [*]  subterm_1200  rparen "
   };
  return s;
 }

 std::vector<int> state_la_42() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state43() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_43() const {
  std::vector<std::string> s{
   "term_0 :-  string  [*] "
   };
  return s;
 }

 std::vector<int> state_la_43() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state44() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_44() const {
  std::vector<std::string> s{
   "term_0 :-  variable  [*] "
   };
  return s;
 }

 std::vector<int> state_la_44() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state45() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(236); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_45() const {
  std::vector<std::string> s{
   "term_n :-  op_fx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_45() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state46() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(245); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_46() const {
  std::vector<std::string> s{
   "term_n :-  op_fy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_46() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state47() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF: Base::shift_and_goto_state(237); break;
   case SYMBOL_OP_XFX: Base::shift_and_goto_state(238); break;
   case SYMBOL_OP_XFY: Base::shift_and_goto_state(239); break;
   case SYMBOL_OP_YF: Base::shift_and_goto_state(240); break;
   case SYMBOL_OP_YFX: Base::shift_and_goto_state(241); break;
   case SYMBOL_COMMA:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__subterm_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_47() const {
  std::vector<std::string> s{
   "subterm_999 :-  subterm_n  [*] ", 
   "term_n      :-                  op_xf  ", 
   "             |                  op_yf  ", 
   "             |                  op_xfx  subterm_n ", 
   "             |                  op_xfy            ", 
   "             |                  op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_47() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state48() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__term_0(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_48() const {
  std::vector<std::string> s{
   "term_n :-  term_0  [*] "
   };
  return s;
 }

 std::vector<int> state_la_48() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state49() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_49() const {
  std::vector<std::string> s{
   "unsigned_number :-  natural_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_49() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state50() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_50() const {
  std::vector<std::string> s{
   "unsigned_number :-  unsigned_float  [*] "
   };
  return s;
 }

 std::vector<int> state_la_50() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state51() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(53); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_51() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  [*]  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_51() const {
  std::vector<int> s{SYMBOL_RBRACKET};
  return s;
 }

 void state52() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_52() const {
  std::vector<std::string> s{
   "list :-  lbracket  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_52() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state53() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_53() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_53() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state54() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_54() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_54() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state55() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(57); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(33); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(47); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_55() const {
  std::vector<std::string> s{
   "listexpr :-  subterm_999  comma  [*]  listexpr "
   };
  return s;
 }

 std::vector<int> state_la_55() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LISTEXPR, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state56() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(58); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(59); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(70); break;
   case SYMBOL_INF: Base::shift_and_goto_state(65); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(71); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(63); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(73); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(60); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(80); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(76); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(77); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(74); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(81); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(75); break;
   case SYMBOL_ATOM: Base::goto_state(61); break;
   case SYMBOL_CONSTANT: Base::goto_state(69); break;
   case SYMBOL_LIST: Base::goto_state(72); break;
   case SYMBOL_NUMBER: Base::goto_state(62); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(64); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(78); break;
   case SYMBOL_TERM_0: Base::goto_state(79); break;
   case SYMBOL_TERM_N: Base::goto_state(68); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(67); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_56() const {
  std::vector<std::string> s{
   "listexpr :-  subterm_999  vbar  [*]  subterm_999 "
   };
  return s;
 }

 std::vector<int> state_la_56() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state57() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LISTEXPR, Base::reduce_listexpr__subterm_999_comma_listexpr(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_57() const {
  std::vector<std::string> s{
   "listexpr :-  subterm_999  comma  listexpr  [*] "
   };
  return s;
 }

 std::vector<int> state_la_57() const {
  std::vector<int> s{SYMBOL_RBRACKET};
  return s;
 }

 void state58() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_brace(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_58() const {
  std::vector<std::string> s{
   "atom :-  empty_brace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_58() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state59() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_59() const {
  std::vector<std::string> s{
   "atom :-  empty_list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_59() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state60() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_60() const {
  std::vector<std::string> s{
   "atom :-  name  [*] "
   };
  return s;
 }

 std::vector<int> state_la_60() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state61() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_61() const {
  std::vector<std::string> s{
   "constant :-  atom  [*] "
   };
  return s;
 }

 std::vector<int> state_la_61() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state62() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_62() const {
  std::vector<std::string> s{
   "constant :-  number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_62() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state63() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(83); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(82); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(33); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(47); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_63() const {
  std::vector<std::string> s{
   "list :-  lbracket  [*]  rbracket ", 
   "      |                 listexpr  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_63() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LISTEXPR, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_RBRACKET, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state64() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LISTEXPR, Base::reduce_listexpr__subterm_999_vbar_subterm_999(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_64() const {
  std::vector<std::string> s{
   "listexpr :-  subterm_999  vbar  subterm_999  [*] "
   };
  return s;
 }

 std::vector<int> state_la_64() const {
  std::vector<int> s{SYMBOL_RBRACKET};
  return s;
 }

 void state65() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_65() const {
  std::vector<std::string> s{
   "number :-  inf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_65() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state66() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_66() const {
  std::vector<std::string> s{
   "number :-  nan  [*] "
   };
  return s;
 }

 std::vector<int> state_la_66() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state67() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_67() const {
  std::vector<std::string> s{
   "number :-  unsigned_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_67() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state68() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_SUBTERM_N, Base::reduce_subterm_n__term_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_68() const {
  std::vector<std::string> s{
   "subterm_n :-  term_n  [*] "
   };
  return s;
 }

 std::vector<int> state_la_68() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state69() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_69() const {
  std::vector<std::string> s{
   "term_0 :-  constant  [*] "
   };
  return s;
 }

 std::vector<int> state_la_69() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state70() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(103); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(85); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(106); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_70() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  [*]  arguments  rparen "
   };
  return s;
 }

 std::vector<int> state_la_70() const {
  std::vector<int> s{SYMBOL_ARGUMENTS, SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state71() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(216); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_71() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  [*]  subterm_1200  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_71() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state72() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_72() const {
  std::vector<std::string> s{
   "term_0 :-  list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_72() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state73() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(218); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(168); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_73() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  [*]  subterm_1200  rparen "
   };
  return s;
 }

 std::vector<int> state_la_73() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state74() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_74() const {
  std::vector<std::string> s{
   "term_0 :-  string  [*] "
   };
  return s;
 }

 std::vector<int> state_la_74() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state75() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_75() const {
  std::vector<std::string> s{
   "term_0 :-  variable  [*] "
   };
  return s;
 }

 std::vector<int> state_la_75() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state76() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(58); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(59); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(70); break;
   case SYMBOL_INF: Base::shift_and_goto_state(65); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(71); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(63); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(73); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(60); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(80); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(76); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(77); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(74); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(81); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(75); break;
   case SYMBOL_ATOM: Base::goto_state(61); break;
   case SYMBOL_CONSTANT: Base::goto_state(69); break;
   case SYMBOL_LIST: Base::goto_state(72); break;
   case SYMBOL_NUMBER: Base::goto_state(62); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(220); break;
   case SYMBOL_TERM_0: Base::goto_state(79); break;
   case SYMBOL_TERM_N: Base::goto_state(68); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(67); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_76() const {
  std::vector<std::string> s{
   "term_n :-  op_fx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_76() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state77() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(58); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(59); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(70); break;
   case SYMBOL_INF: Base::shift_and_goto_state(65); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(71); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(63); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(73); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(60); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(80); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(76); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(77); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(74); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(81); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(75); break;
   case SYMBOL_ATOM: Base::goto_state(61); break;
   case SYMBOL_CONSTANT: Base::goto_state(69); break;
   case SYMBOL_LIST: Base::goto_state(72); break;
   case SYMBOL_NUMBER: Base::goto_state(62); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(229); break;
   case SYMBOL_TERM_0: Base::goto_state(79); break;
   case SYMBOL_TERM_N: Base::goto_state(68); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(67); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_77() const {
  std::vector<std::string> s{
   "term_n :-  op_fy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_77() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state78() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF: Base::shift_and_goto_state(221); break;
   case SYMBOL_OP_XFX: Base::shift_and_goto_state(222); break;
   case SYMBOL_OP_XFY: Base::shift_and_goto_state(223); break;
   case SYMBOL_OP_YF: Base::shift_and_goto_state(224); break;
   case SYMBOL_OP_YFX: Base::shift_and_goto_state(225); break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__subterm_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_78() const {
  std::vector<std::string> s{
   "subterm_999 :-  subterm_n  [*] ", 
   "term_n      :-                  op_xf  ", 
   "             |                  op_yf  ", 
   "             |                  op_xfx  subterm_n ", 
   "             |                  op_xfy            ", 
   "             |                  op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_78() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state79() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__term_0(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_79() const {
  std::vector<std::string> s{
   "term_n :-  term_0  [*] "
   };
  return s;
 }

 std::vector<int> state_la_79() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state80() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_80() const {
  std::vector<std::string> s{
   "unsigned_number :-  natural_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_80() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state81() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_81() const {
  std::vector<std::string> s{
   "unsigned_number :-  unsigned_float  [*] "
   };
  return s;
 }

 std::vector<int> state_la_81() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state82() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(84); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_82() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  [*]  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_82() const {
  std::vector<int> s{SYMBOL_RBRACKET};
  return s;
 }

 void state83() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_83() const {
  std::vector<std::string> s{
   "list :-  lbracket  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_83() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state84() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_84() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_84() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state85() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA: Base::shift_and_goto_state(110); break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ARGUMENTS, Base::reduce_arguments__subterm_999(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_85() const {
  std::vector<std::string> s{
   "arguments :-  subterm_999  [*] ", 
   "           |                    comma  arguments "
   };
  return s;
 }

 std::vector<int> state_la_85() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_RPAREN};
  return s;
 }

 void state86() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_brace(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_86() const {
  std::vector<std::string> s{
   "atom :-  empty_brace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_86() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state87() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_87() const {
  std::vector<std::string> s{
   "atom :-  empty_list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_87() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state88() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_88() const {
  std::vector<std::string> s{
   "atom :-  name  [*] "
   };
  return s;
 }

 std::vector<int> state_la_88() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state89() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_89() const {
  std::vector<std::string> s{
   "constant :-  atom  [*] "
   };
  return s;
 }

 std::vector<int> state_la_89() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state90() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_90() const {
  std::vector<std::string> s{
   "constant :-  number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_90() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state91() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(113); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(112); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(33); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(47); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_91() const {
  std::vector<std::string> s{
   "list :-  lbracket  [*]  rbracket ", 
   "      |                 listexpr  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_91() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LISTEXPR, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_RBRACKET, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state92() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_92() const {
  std::vector<std::string> s{
   "number :-  inf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_92() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state93() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_93() const {
  std::vector<std::string> s{
   "number :-  nan  [*] "
   };
  return s;
 }

 std::vector<int> state_la_93() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state94() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_94() const {
  std::vector<std::string> s{
   "number :-  unsigned_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_94() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state95() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_N, Base::reduce_subterm_n__term_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_95() const {
  std::vector<std::string> s{
   "subterm_n :-  term_n  [*] "
   };
  return s;
 }

 std::vector<int> state_la_95() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state96() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_96() const {
  std::vector<std::string> s{
   "term_0 :-  constant  [*] "
   };
  return s;
 }

 std::vector<int> state_la_96() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state97() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(115); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(85); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(106); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_97() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  [*]  arguments  rparen "
   };
  return s;
 }

 std::vector<int> state_la_97() const {
  std::vector<int> s{SYMBOL_ARGUMENTS, SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state98() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(134); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_98() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  [*]  subterm_1200  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_98() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state99() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_99() const {
  std::vector<std::string> s{
   "term_0 :-  list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_99() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state100() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(203); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(168); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_100() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  [*]  subterm_1200  rparen "
   };
  return s;
 }

 std::vector<int> state_la_100() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state101() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_101() const {
  std::vector<std::string> s{
   "term_0 :-  string  [*] "
   };
  return s;
 }

 std::vector<int> state_la_101() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state102() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_102() const {
  std::vector<std::string> s{
   "term_0 :-  variable  [*] "
   };
  return s;
 }

 std::vector<int> state_la_102() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state103() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(205); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_103() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_103() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state104() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(206); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_104() const {
  std::vector<std::string> s{
   "term_n :-  op_fx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_104() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state105() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(215); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_105() const {
  std::vector<std::string> s{
   "term_n :-  op_fy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_105() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state106() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF: Base::shift_and_goto_state(207); break;
   case SYMBOL_OP_XFX: Base::shift_and_goto_state(208); break;
   case SYMBOL_OP_XFY: Base::shift_and_goto_state(209); break;
   case SYMBOL_OP_YF: Base::shift_and_goto_state(210); break;
   case SYMBOL_OP_YFX: Base::shift_and_goto_state(211); break;
   case SYMBOL_COMMA:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_999, Base::reduce_subterm_999__subterm_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_106() const {
  std::vector<std::string> s{
   "subterm_999 :-  subterm_n  [*] ", 
   "term_n      :-                  op_xf  ", 
   "             |                  op_yf  ", 
   "             |                  op_xfx  subterm_n ", 
   "             |                  op_xfy            ", 
   "             |                  op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_106() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state107() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__term_0(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_107() const {
  std::vector<std::string> s{
   "term_n :-  term_0  [*] "
   };
  return s;
 }

 std::vector<int> state_la_107() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state108() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_108() const {
  std::vector<std::string> s{
   "unsigned_number :-  natural_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_108() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state109() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_109() const {
  std::vector<std::string> s{
   "unsigned_number :-  unsigned_float  [*] "
   };
  return s;
 }

 std::vector<int> state_la_109() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state110() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(111); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(85); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(106); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_110() const {
  std::vector<std::string> s{
   "arguments :-  subterm_999  comma  [*]  arguments "
   };
  return s;
 }

 std::vector<int> state_la_110() const {
  std::vector<int> s{SYMBOL_ARGUMENTS, SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state111() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ARGUMENTS, Base::reduce_arguments__subterm_999_comma_arguments(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_111() const {
  std::vector<std::string> s{
   "arguments :-  subterm_999  comma  arguments  [*] "
   };
  return s;
 }

 std::vector<int> state_la_111() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state112() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(114); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_112() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  [*]  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_112() const {
  std::vector<int> s{SYMBOL_RBRACKET};
  return s;
 }

 void state113() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_113() const {
  std::vector<std::string> s{
   "list :-  lbracket  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_113() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state114() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_114() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_114() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state115() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(116); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_115() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_115() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state116() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_116() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_116() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state117() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_brace(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_117() const {
  std::vector<std::string> s{
   "atom :-  empty_brace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_117() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state118() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_118() const {
  std::vector<std::string> s{
   "atom :-  empty_list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_118() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state119() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_119() const {
  std::vector<std::string> s{
   "atom :-  name  [*] "
   };
  return s;
 }

 std::vector<int> state_la_119() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state120() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_120() const {
  std::vector<std::string> s{
   "constant :-  atom  [*] "
   };
  return s;
 }

 std::vector<int> state_la_120() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state121() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_121() const {
  std::vector<std::string> s{
   "constant :-  number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_121() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state122() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(142); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(141); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(33); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(47); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_122() const {
  std::vector<std::string> s{
   "list :-  lbracket  [*]  rbracket ", 
   "      |                 listexpr  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_122() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LISTEXPR, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_RBRACKET, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state123() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_123() const {
  std::vector<std::string> s{
   "number :-  inf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_123() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state124() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_124() const {
  std::vector<std::string> s{
   "number :-  nan  [*] "
   };
  return s;
 }

 std::vector<int> state_la_124() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state125() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_125() const {
  std::vector<std::string> s{
   "number :-  unsigned_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_125() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state126() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_N, Base::reduce_subterm_n__term_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_126() const {
  std::vector<std::string> s{
   "subterm_n :-  term_n  [*] "
   };
  return s;
 }

 std::vector<int> state_la_126() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state127() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_127() const {
  std::vector<std::string> s{
   "term_0 :-  constant  [*] "
   };
  return s;
 }

 std::vector<int> state_la_127() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state128() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(144); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(85); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(106); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_128() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  [*]  arguments  rparen "
   };
  return s;
 }

 std::vector<int> state_la_128() const {
  std::vector<int> s{SYMBOL_ARGUMENTS, SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state129() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(146); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_129() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  [*]  subterm_1200  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_129() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state130() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_130() const {
  std::vector<std::string> s{
   "term_0 :-  list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_130() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state131() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(165); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(168); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_131() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  [*]  subterm_1200  rparen "
   };
  return s;
 }

 std::vector<int> state_la_131() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state132() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_132() const {
  std::vector<std::string> s{
   "term_0 :-  string  [*] "
   };
  return s;
 }

 std::vector<int> state_la_132() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state133() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_133() const {
  std::vector<std::string> s{
   "term_0 :-  variable  [*] "
   };
  return s;
 }

 std::vector<int> state_la_133() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state134() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(192); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_134() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  [*]  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_134() const {
  std::vector<int> s{SYMBOL_RBRACE};
  return s;
 }

 void state135() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(193); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_135() const {
  std::vector<std::string> s{
   "term_n :-  op_fx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_135() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state136() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(202); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_136() const {
  std::vector<std::string> s{
   "term_n :-  op_fy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_136() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state137() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF: Base::shift_and_goto_state(194); break;
   case SYMBOL_OP_XFX: Base::shift_and_goto_state(195); break;
   case SYMBOL_OP_XFY: Base::shift_and_goto_state(196); break;
   case SYMBOL_OP_YF: Base::shift_and_goto_state(197); break;
   case SYMBOL_OP_YFX: Base::shift_and_goto_state(198); break;
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__subterm_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_137() const {
  std::vector<std::string> s{
   "subterm_1200 :-  subterm_n  [*] ", 
   "term_n       :-                  op_xf  ", 
   "              |                  op_yf  ", 
   "              |                  op_xfx  subterm_n ", 
   "              |                  op_xfy            ", 
   "              |                  op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_137() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state138() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__term_0(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_138() const {
  std::vector<std::string> s{
   "term_n :-  term_0  [*] "
   };
  return s;
 }

 std::vector<int> state_la_138() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state139() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_139() const {
  std::vector<std::string> s{
   "unsigned_number :-  natural_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_139() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state140() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_140() const {
  std::vector<std::string> s{
   "unsigned_number :-  unsigned_float  [*] "
   };
  return s;
 }

 std::vector<int> state_la_140() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state141() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(143); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_141() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  [*]  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_141() const {
  std::vector<int> s{SYMBOL_RBRACKET};
  return s;
 }

 void state142() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_142() const {
  std::vector<std::string> s{
   "list :-  lbracket  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_142() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state143() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_143() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_143() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state144() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(145); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_144() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_144() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state145() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_145() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_145() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state146() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(147); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_146() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  [*]  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_146() const {
  std::vector<int> s{SYMBOL_RBRACE};
  return s;
 }

 void state147() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_147() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  rbrace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_147() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state148() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_brace(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_148() const {
  std::vector<std::string> s{
   "atom :-  empty_brace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_148() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state149() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__empty_list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_149() const {
  std::vector<std::string> s{
   "atom :-  empty_list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_149() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state150() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_ATOM, Base::reduce_atom__name(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_150() const {
  std::vector<std::string> s{
   "atom :-  name  [*] "
   };
  return s;
 }

 std::vector<int> state_la_150() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state151() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__atom(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_151() const {
  std::vector<std::string> s{
   "constant :-  atom  [*] "
   };
  return s;
 }

 std::vector<int> state_la_151() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state152() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_CONSTANT, Base::reduce_constant__number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_152() const {
  std::vector<std::string> s{
   "constant :-  number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_152() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state153() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(173); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_LISTEXPR: Base::goto_state(172); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(33); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(47); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_153() const {
  std::vector<std::string> s{
   "list :-  lbracket  [*]  rbracket ", 
   "      |                 listexpr  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_153() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LISTEXPR, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_RBRACKET, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state154() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__inf(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_154() const {
  std::vector<std::string> s{
   "number :-  inf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_154() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state155() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__nan(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_155() const {
  std::vector<std::string> s{
   "number :-  nan  [*] "
   };
  return s;
 }

 std::vector<int> state_la_155() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state156() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_NUMBER, Base::reduce_number__unsigned_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_156() const {
  std::vector<std::string> s{
   "number :-  unsigned_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_156() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state157() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_N, Base::reduce_subterm_n__term_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_157() const {
  std::vector<std::string> s{
   "subterm_n :-  term_n  [*] "
   };
  return s;
 }

 std::vector<int> state_la_157() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state158() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__constant(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_158() const {
  std::vector<std::string> s{
   "term_0 :-  constant  [*] "
   };
  return s;
 }

 std::vector<int> state_la_158() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state159() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ARGUMENTS: Base::goto_state(175); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_999: Base::goto_state(85); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(106); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_159() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  [*]  arguments  rparen "
   };
  return s;
 }

 std::vector<int> state_la_159() const {
  std::vector<int> s{SYMBOL_ARGUMENTS, SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_999, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state160() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(177); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(137); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_160() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  [*]  subterm_1200  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_160() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state161() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__list(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_161() const {
  std::vector<std::string> s{
   "term_0 :-  list  [*] "
   };
  return s;
 }

 std::vector<int> state_la_161() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state162() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_1200: Base::goto_state(179); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(168); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_162() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  [*]  subterm_1200  rparen "
   };
  return s;
 }

 std::vector<int> state_la_162() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_1200, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state163() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__string(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_163() const {
  std::vector<std::string> s{
   "term_0 :-  string  [*] "
   };
  return s;
 }

 std::vector<int> state_la_163() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state164() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__variable(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_164() const {
  std::vector<std::string> s{
   "term_0 :-  variable  [*] "
   };
  return s;
 }

 std::vector<int> state_la_164() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state165() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(181); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_165() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_165() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state166() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(182); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_166() const {
  std::vector<std::string> s{
   "term_n :-  op_fx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_166() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state167() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(191); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_167() const {
  std::vector<std::string> s{
   "term_n :-  op_fy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_167() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state168() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF: Base::shift_and_goto_state(183); break;
   case SYMBOL_OP_XFX: Base::shift_and_goto_state(184); break;
   case SYMBOL_OP_XFY: Base::shift_and_goto_state(185); break;
   case SYMBOL_OP_YF: Base::shift_and_goto_state(186); break;
   case SYMBOL_OP_YFX: Base::shift_and_goto_state(187); break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_SUBTERM_1200, Base::reduce_subterm_1200__subterm_n(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_168() const {
  std::vector<std::string> s{
   "subterm_1200 :-  subterm_n  [*] ", 
   "term_n       :-                  op_xf  ", 
   "              |                  op_yf  ", 
   "              |                  op_xfx  subterm_n ", 
   "              |                  op_xfy            ", 
   "              |                  op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_168() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state169() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__term_0(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_169() const {
  std::vector<std::string> s{
   "term_n :-  term_0  [*] "
   };
  return s;
 }

 std::vector<int> state_la_169() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state170() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__natural_number(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_170() const {
  std::vector<std::string> s{
   "unsigned_number :-  natural_number  [*] "
   };
  return s;
 }

 std::vector<int> state_la_170() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state171() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_UNSIGNED_NUMBER, Base::reduce_unsigned_number__unsigned_float(Base::args(1)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_171() const {
  std::vector<std::string> s{
   "unsigned_number :-  unsigned_float  [*] "
   };
  return s;
 }

 std::vector<int> state_la_171() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state172() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACKET: Base::shift_and_goto_state(174); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_172() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  [*]  rbracket "
   };
  return s;
 }

 std::vector<int> state_la_172() const {
  std::vector<int> s{SYMBOL_RBRACKET};
  return s;
 }

 void state173() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_rbracket(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_173() const {
  std::vector<std::string> s{
   "list :-  lbracket  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_173() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state174() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_LIST, Base::reduce_list__lbracket_listexpr_rbracket(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_174() const {
  std::vector<std::string> s{
   "list :-  lbracket  listexpr  rbracket  [*] "
   };
  return s;
 }

 std::vector<int> state_la_174() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state175() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(176); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_175() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_175() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state176() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_176() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_176() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state177() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(178); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_177() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  [*]  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_177() const {
  std::vector<int> s{SYMBOL_RBRACE};
  return s;
 }

 void state178() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_178() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  rbrace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_178() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state179() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(180); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_179() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_179() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state180() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_180() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_180() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state181() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_181() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_181() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state182() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(183);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(184);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(185);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(186);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(187);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_182() const {
  std::vector<std::string> s{
   "term_n :-  op_fx      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_182() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state183() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_183() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_183() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state184() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(188); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_184() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_184() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state185() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(189); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_185() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_185() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state186() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_186() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_186() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state187() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(148); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(149); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(159); break;
   case SYMBOL_INF: Base::shift_and_goto_state(154); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(160); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(153); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(162); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(150); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(155); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(170); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(166); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(167); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(163); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(171); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(164); break;
   case SYMBOL_ATOM: Base::goto_state(151); break;
   case SYMBOL_CONSTANT: Base::goto_state(158); break;
   case SYMBOL_LIST: Base::goto_state(161); break;
   case SYMBOL_NUMBER: Base::goto_state(152); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(190); break;
   case SYMBOL_TERM_0: Base::goto_state(169); break;
   case SYMBOL_TERM_N: Base::goto_state(157); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(156); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_187() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_187() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state188() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(183);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(184);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(185);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(186);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(187);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_188() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_188() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state189() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(183);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(184);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(185);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(186);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(187);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_189() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfy  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_189() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state190() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(183);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(184);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(185);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(186);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(187);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_190() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_yfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_190() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state191() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(183);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(184);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(185);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(186);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(187);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_191() const {
  std::vector<std::string> s{
   "term_n :-  op_fy      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_191() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state192() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_192() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  rbrace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_192() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state193() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(194);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(195);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(196);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(197);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(198);
    }
    break;
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_193() const {
  std::vector<std::string> s{
   "term_n :-  op_fx      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_193() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state194() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_194() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_194() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state195() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(199); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_195() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_195() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state196() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(200); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_196() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_196() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state197() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_197() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_197() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state198() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(117); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(118); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(128); break;
   case SYMBOL_INF: Base::shift_and_goto_state(123); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(129); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(122); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(131); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(119); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(124); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(139); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(135); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(136); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(132); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(140); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(133); break;
   case SYMBOL_ATOM: Base::goto_state(120); break;
   case SYMBOL_CONSTANT: Base::goto_state(127); break;
   case SYMBOL_LIST: Base::goto_state(130); break;
   case SYMBOL_NUMBER: Base::goto_state(121); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(201); break;
   case SYMBOL_TERM_0: Base::goto_state(138); break;
   case SYMBOL_TERM_N: Base::goto_state(126); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(125); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_198() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_198() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state199() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(194);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(195);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(196);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(197);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(198);
    }
    break;
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_199() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_199() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state200() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(194);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(195);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(196);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(197);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(198);
    }
    break;
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_200() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfy  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_200() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state201() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(194);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(195);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(196);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(197);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(198);
    }
    break;
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_201() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_yfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_201() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state202() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(194);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(195);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(196);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(197);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(198);
    }
    break;
   case SYMBOL_RBRACE:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_202() const {
  std::vector<std::string> s{
   "term_n :-  op_fy      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_202() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACE};
  return s;
 }

 void state203() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(204); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_203() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_203() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state204() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_204() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_204() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state205() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_205() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_205() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state206() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(207);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(208);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(209);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(210);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(211);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_206() const {
  std::vector<std::string> s{
   "term_n :-  op_fx      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_206() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state207() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_207() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_207() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state208() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(212); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_208() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_208() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state209() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(213); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_209() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_209() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state210() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_210() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_210() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state211() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(86); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(87); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(97); break;
   case SYMBOL_INF: Base::shift_and_goto_state(92); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(98); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(91); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(100); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(88); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(93); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(108); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(104); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(105); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(101); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(109); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(102); break;
   case SYMBOL_ATOM: Base::goto_state(89); break;
   case SYMBOL_CONSTANT: Base::goto_state(96); break;
   case SYMBOL_LIST: Base::goto_state(99); break;
   case SYMBOL_NUMBER: Base::goto_state(90); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(214); break;
   case SYMBOL_TERM_0: Base::goto_state(107); break;
   case SYMBOL_TERM_N: Base::goto_state(95); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(94); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_211() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_211() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state212() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(207);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(208);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(209);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(210);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(211);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_212() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_212() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state213() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(207);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(208);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(209);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(210);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(211);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_213() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfy  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_213() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state214() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(207);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(208);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(209);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(210);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(211);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_214() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_yfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_214() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state215() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(207);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(208);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(209);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(210);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(211);
    }
    break;
   case SYMBOL_RPAREN:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_215() const {
  std::vector<std::string> s{
   "term_n :-  op_fy      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_215() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RPAREN};
  return s;
 }

 void state216() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(217); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_216() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  [*]  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_216() const {
  std::vector<int> s{SYMBOL_RBRACE};
  return s;
 }

 void state217() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_217() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  rbrace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_217() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state218() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(219); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_218() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_218() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state219() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_219() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_219() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state220() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(221);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(222);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(223);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(224);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(225);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_220() const {
  std::vector<std::string> s{
   "term_n :-  op_fx      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_220() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state221() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_221() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_221() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state222() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(58); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(59); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(70); break;
   case SYMBOL_INF: Base::shift_and_goto_state(65); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(71); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(63); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(73); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(60); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(80); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(76); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(77); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(74); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(81); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(75); break;
   case SYMBOL_ATOM: Base::goto_state(61); break;
   case SYMBOL_CONSTANT: Base::goto_state(69); break;
   case SYMBOL_LIST: Base::goto_state(72); break;
   case SYMBOL_NUMBER: Base::goto_state(62); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(226); break;
   case SYMBOL_TERM_0: Base::goto_state(79); break;
   case SYMBOL_TERM_N: Base::goto_state(68); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(67); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_222() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_222() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state223() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(58); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(59); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(70); break;
   case SYMBOL_INF: Base::shift_and_goto_state(65); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(71); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(63); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(73); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(60); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(80); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(76); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(77); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(74); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(81); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(75); break;
   case SYMBOL_ATOM: Base::goto_state(61); break;
   case SYMBOL_CONSTANT: Base::goto_state(69); break;
   case SYMBOL_LIST: Base::goto_state(72); break;
   case SYMBOL_NUMBER: Base::goto_state(62); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(227); break;
   case SYMBOL_TERM_0: Base::goto_state(79); break;
   case SYMBOL_TERM_N: Base::goto_state(68); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(67); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_223() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_223() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state224() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_224() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_224() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state225() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(58); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(59); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(70); break;
   case SYMBOL_INF: Base::shift_and_goto_state(65); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(71); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(63); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(73); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(60); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(66); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(80); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(76); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(77); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(74); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(81); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(75); break;
   case SYMBOL_ATOM: Base::goto_state(61); break;
   case SYMBOL_CONSTANT: Base::goto_state(69); break;
   case SYMBOL_LIST: Base::goto_state(72); break;
   case SYMBOL_NUMBER: Base::goto_state(62); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(228); break;
   case SYMBOL_TERM_0: Base::goto_state(79); break;
   case SYMBOL_TERM_N: Base::goto_state(68); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(67); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_225() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_225() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state226() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(221);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(222);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(223);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(224);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(225);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_226() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_226() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state227() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(221);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(222);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(223);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(224);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(225);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_227() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfy  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_227() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state228() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(221);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(222);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(223);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(224);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(225);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_228() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_yfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_228() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state229() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_OP_XF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(221);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(222);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(223);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(224);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(225);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_229() const {
  std::vector<std::string> s{
   "term_n :-  op_fy      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_229() const {
  std::vector<int> s{SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET};
  return s;
 }

 void state230() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(231); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_230() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_230() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state231() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_231() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_231() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state232() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(233); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_232() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  [*]  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_232() const {
  std::vector<int> s{SYMBOL_RBRACE};
  return s;
 }

 void state233() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_233() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  rbrace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_233() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state234() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(235); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_234() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_234() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state235() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
   case SYMBOL_RBRACKET:
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_235() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_235() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state236() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(237);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(238);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(239);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(240);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(241);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_236() const {
  std::vector<std::string> s{
   "term_n :-  op_fx      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_236() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state237() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_237() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_237() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state238() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(242); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_238() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_238() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state239() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(243); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_239() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_239() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state240() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_240() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_240() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state241() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(25); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(26); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(39); break;
   case SYMBOL_INF: Base::shift_and_goto_state(34); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(40); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(30); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(42); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(27); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(35); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(49); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(45); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(46); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(43); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(50); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(44); break;
   case SYMBOL_ATOM: Base::goto_state(28); break;
   case SYMBOL_CONSTANT: Base::goto_state(38); break;
   case SYMBOL_LIST: Base::goto_state(41); break;
   case SYMBOL_NUMBER: Base::goto_state(29); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(244); break;
   case SYMBOL_TERM_0: Base::goto_state(48); break;
   case SYMBOL_TERM_N: Base::goto_state(37); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(36); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_241() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_241() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state242() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(237);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(238);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(239);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(240);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(241);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_242() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_242() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state243() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(237);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(238);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(239);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(240);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(241);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_243() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfy  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_243() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state244() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(237);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(238);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(239);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(240);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(241);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_244() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_yfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_244() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state245() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_COMMA:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(237);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(238);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(239);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(240);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(241);
    }
    break;
   case SYMBOL_RBRACKET:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   case SYMBOL_VBAR:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_245() const {
  std::vector<std::string> s{
   "term_n :-  op_fy      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_245() const {
  std::vector<int> s{SYMBOL_COMMA, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX, SYMBOL_RBRACKET, SYMBOL_VBAR};
  return s;
 }

 void state246() {
  switch (Base::lookahead().ordinal()) {
   default: 
    Base::reduce(SYMBOL_START, Base::reduce_start__subterm_1200_full_stop(Base::args(2)));
    break;
  }
 }

 std::vector<std::string> state_strings_246() const {
  std::vector<std::string> s{
   "start :-  subterm_1200  full_stop  [*] "
   };
  return s;
 }

 std::vector<int> state_la_246() const {
  std::vector<int> s{SYMBOL_EMPTY};
  return s;
 }

 void state247() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(248); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_247() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_247() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state248() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__functor_lparen_arguments_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_248() const {
  std::vector<std::string> s{
   "term_0 :-  functor_lparen  arguments  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_248() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state249() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RBRACE: Base::shift_and_goto_state(250); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_249() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  [*]  rbrace "
   };
  return s;
 }

 std::vector<int> state_la_249() const {
  std::vector<int> s{SYMBOL_RBRACE};
  return s;
 }

 void state250() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lbrace_subterm_1200_rbrace(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_250() const {
  std::vector<std::string> s{
   "term_0 :-  lbrace  subterm_1200  rbrace  [*] "
   };
  return s;
 }

 std::vector<int> state_la_250() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state251() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_RPAREN: Base::shift_and_goto_state(252); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_251() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  [*]  rparen "
   };
  return s;
 }

 std::vector<int> state_la_251() const {
  std::vector<int> s{SYMBOL_RPAREN};
  return s;
 }

 void state252() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
   case SYMBOL_OP_XF:
   case SYMBOL_OP_XFX:
   case SYMBOL_OP_XFY:
   case SYMBOL_OP_YF:
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_0, Base::reduce_term_0__lparen_subterm_1200_rparen(Base::args(3)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_252() const {
  std::vector<std::string> s{
   "term_0 :-  lparen  subterm_1200  rparen  [*] "
   };
  return s;
 }

 std::vector<int> state_la_252() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state253() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(254);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(255);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(256);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(257);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fx_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(258);
    }
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_253() const {
  std::vector<std::string> s{
   "term_n :-  op_fx      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_253() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state254() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_254() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_254() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state255() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(1); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(2); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(7); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(6); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(8); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(23); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(20); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(24); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(4); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(5); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(259); break;
   case SYMBOL_TERM_0: Base::goto_state(22); break;
   case SYMBOL_TERM_N: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(9); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_255() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_255() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state256() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(1); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(2); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(7); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(6); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(8); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(23); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(20); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(24); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(4); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(5); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(260); break;
   case SYMBOL_TERM_0: Base::goto_state(22); break;
   case SYMBOL_TERM_N: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(9); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_256() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_xfy  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_256() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state257() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_XFY:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YF:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   case SYMBOL_OP_YFX:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yf(Base::args(2)));
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_257() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yf  [*] "
   };
  return s;
 }

 std::vector<int> state_la_257() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state258() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_EMPTY_BRACE: Base::shift_and_goto_state(1); break;
   case SYMBOL_EMPTY_LIST: Base::shift_and_goto_state(2); break;
   case SYMBOL_FUNCTOR_LPAREN: Base::shift_and_goto_state(13); break;
   case SYMBOL_INF: Base::shift_and_goto_state(7); break;
   case SYMBOL_LBRACE: Base::shift_and_goto_state(14); break;
   case SYMBOL_LBRACKET: Base::shift_and_goto_state(6); break;
   case SYMBOL_LPAREN: Base::shift_and_goto_state(16); break;
   case SYMBOL_NAME: Base::shift_and_goto_state(3); break;
   case SYMBOL_NAN: Base::shift_and_goto_state(8); break;
   case SYMBOL_NATURAL_NUMBER: Base::shift_and_goto_state(23); break;
   case SYMBOL_OP_FX: Base::shift_and_goto_state(19); break;
   case SYMBOL_OP_FY: Base::shift_and_goto_state(20); break;
   case SYMBOL_STRING: Base::shift_and_goto_state(17); break;
   case SYMBOL_UNSIGNED_FLOAT: Base::shift_and_goto_state(24); break;
   case SYMBOL_VARIABLE: Base::shift_and_goto_state(18); break;
   case SYMBOL_ATOM: Base::goto_state(4); break;
   case SYMBOL_CONSTANT: Base::goto_state(12); break;
   case SYMBOL_LIST: Base::goto_state(15); break;
   case SYMBOL_NUMBER: Base::goto_state(5); break;
   case SYMBOL_SUBTERM_N: Base::goto_state(261); break;
   case SYMBOL_TERM_0: Base::goto_state(22); break;
   case SYMBOL_TERM_N: Base::goto_state(11); break;
   case SYMBOL_UNSIGNED_NUMBER: Base::goto_state(9); break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_258() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  op_yfx  [*]  subterm_n "
   };
  return s;
 }

 std::vector<int> state_la_258() const {
  std::vector<int> s{SYMBOL_ATOM, SYMBOL_CONSTANT, SYMBOL_EMPTY_BRACE, SYMBOL_EMPTY_LIST, SYMBOL_FUNCTOR_LPAREN, SYMBOL_INF, SYMBOL_LBRACE, SYMBOL_LBRACKET, SYMBOL_LIST, SYMBOL_LPAREN, SYMBOL_NAME, SYMBOL_NAN, SYMBOL_NATURAL_NUMBER, SYMBOL_NUMBER, SYMBOL_OP_FX, SYMBOL_OP_FY, SYMBOL_STRING, SYMBOL_SUBTERM_N, SYMBOL_TERM_0, SYMBOL_TERM_N, SYMBOL_UNSIGNED_FLOAT, SYMBOL_UNSIGNED_NUMBER, SYMBOL_VARIABLE};
  return s;
 }

 void state259() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(254);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(255);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(256);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(257);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(258);
    }
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_259() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_259() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state260() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(254);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(255);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(256);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(257);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_xfy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_xfy_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(258);
    }
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_260() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_xfy  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_260() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state261() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(254);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(255);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(256);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(257);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_yfx(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__subterm_n_op_yfx_subterm_n(Base::args(3)));
    } else {
     Base::shift_and_goto_state(258);
    }
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_261() const {
  std::vector<std::string> s{
   "term_n :-  subterm_n  [*]     op_xf     ", 
   "        |                     op_yf     ", 
   "        |                     op_xfx     subterm_n ", 
   "        |                     op_xfy               ", 
   "        |                     op_yfx               ", 
   "        |             op_yfx  subterm_n  [*]       "
   };
  return s;
 }

 std::vector<int> state_la_261() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

 void state262() {
  switch (Base::lookahead().ordinal()) {
   case SYMBOL_FULL_STOP:
    Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    break;
   case SYMBOL_OP_XF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(254);
    }
    break;
   case SYMBOL_OP_XFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(255);
    }
    break;
   case SYMBOL_OP_XFY:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(256);
    }
    break;
   case SYMBOL_OP_YF:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(257);
    }
    break;
   case SYMBOL_OP_YFX:
    if (Base::check_op_fy(Base::lookahead())) {
     Base::reduce(SYMBOL_TERM_N, Base::reduce_term_n__op_fy_subterm_n(Base::args(2)));
    } else {
     Base::shift_and_goto_state(258);
    }
    break;
   default: Base::parse_error(state_description(), state_next_symbols()); break;
  }
 }

 std::vector<std::string> state_strings_262() const {
  std::vector<std::string> s{
   "term_n :-  op_fy      subterm_n  [*]    ", 
   "        |  subterm_n  [*]        op_xf  ", 
   "        |                        op_yf  ", 
   "        |                        op_xfx  subterm_n ", 
   "        |                        op_xfy            ", 
   "        |                        op_yfx            "
   };
  return s;
 }

 std::vector<int> state_la_262() const {
  std::vector<int> s{SYMBOL_FULL_STOP, SYMBOL_OP_XF, SYMBOL_OP_XFX, SYMBOL_OP_XFY, SYMBOL_OP_YF, SYMBOL_OP_YFX};
  return s;
 }

};

} } 
