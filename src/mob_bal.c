

/*
   data for balancing mobiles in NARAI
   by atre@paradise
 */

#define MaxArmor                        100

double HitrollFactor = 		.0007;
double DamrollFactor =		.0015;
double ArmorFactor = 		.0045;

int BaseExp = 80;
int BaseHit = 14;
/*
   data file for mobile balancing in NARAI MUD.
   by atre
 */

/* HIT data for mobile balancing */
int mob_bal_hit[43][20] =
{
    {95, 114, 135, 159, 186,
     215, 247, 282, 321, 363,
     408, 457, 510, 567, 627,
     692, 762, 835, 914, 997},
    {95, 114, 135, 159, 186,
     215, 247, 282, 321, 363,
     408, 457, 510, 567, 627,
     692, 762, 835, 914, 997},
    {228, 271, 319, 372, 430,
     495, 565, 642, 726, 817,
     915, 1021, 1134, 1255, 1385,
     1524, 1671, 1828, 1994, 2170},
    {228, 271, 319, 372, 430,
     495, 565, 642, 726, 817,
     915, 1021, 1134, 1255, 1385,
     1524, 1671, 1828, 1994, 2170},
    {406, 478, 558, 646, 742,
     848, 964, 1090, 1226, 1373,
     1531, 1701, 1883, 2078, 2286,
     2507, 2742, 2991, 3255, 3533},
/* 6 level */
    {406, 478, 558, 646, 742,
     848, 964, 1090, 1226, 1373,
     1531, 1701, 1883, 2078, 2286,
     2507, 2742, 2991, 3255, 3533},
    {638, 744, 861, 990, 1131,
     1285, 1453, 1634, 1830, 2042,
     2268, 2511, 2771, 3048, 3342,
     3656, 3988, 4340, 4711, 5104},
    {638, 744, 861, 990, 1131,
     1285, 1453, 1634, 1830, 2042,
     2268, 2511, 2771, 3048, 3342,
     3656, 3988, 4340, 4711, 5104},
    {930, 1076, 1238, 1414, 1607,
     1816, 2043, 2288, 2552, 2835,
     3139, 3464, 3810, 4178, 4570,
     4985, 5425, 5889, 6380, 6897},
    {930, 1076, 1238, 1414, 1607,
     1816, 2043, 2288, 2552, 2835,
     3139, 3464, 3810, 4178, 4570,
     4985, 5425, 5889, 6380, 6897},
/* 11 level */
    {4496, 5035, 5615, 6239, 6906,
     7620, 8382, 9193, 10054, 10968,
     11935, 12957, 14037, 15174, 16372,
     17630, 18952, 20339, 21791, 23311},
    {5492, 6126, 6806, 7534, 8313,
     9144, 10028, 10968, 11965, 13020,
     14135, 15313, 16554, 17860, 19233,
     20675, 22188, 23772, 25430, 27163},
    {6636, 7373, 8162, 9006, 9906,
     10864, 11882, 12962, 14105, 15313,
     16589, 17933, 19348, 20836, 22398,
     24037, 25753, 27549, 29427, 31388},
    {7940, 8790, 9699, 10668, 11700,
     12796, 13959, 15190, 16491, 17865,
     19313, 20837, 22439, 24121, 25886,
     27734, 29668, 31690, 33802, 36006},
    {9418, 10392, 11430, 12536, 13710,
     14956, 16275, 17669, 19141, 20692,
     22325, 24042, 25844, 27735, 29715,
     31787, 33954, 36217, 38578, 41040},
    {11085, 12192, 13371, 14624, 15953,
     17360, 18847, 20417, 22072, 23813,
     25645, 27567, 29584, 31696, 33906,
     36218, 38631, 41150, 43776, 46511},
    {12954, 14207, 15538, 16950, 18445,
     20025, 21693, 23451, 25302, 27247,
     29290, 31433, 33677, 36026, 38481,
     41046, 43722, 46512, 49418, 52443},
    {15043, 16452, 17947, 19530, 21203,
     22969, 24831, 26790, 28850, 31013,
     33282, 35658, 38145, 40745, 43460,
     46294, 49248, 52325, 55528, 58859},
    {17366, 18944, 20615, 22381, 24245,
     26210, 28279, 30453, 32736, 35131,
     37639, 40264, 43008, 45875, 48866,
     51984, 55232, 58613, 62129, 65782},
    {19941, 21700, 23559, 25521, 27590,
     29767, 32056, 34459, 36980, 39620,
     42383, 45272, 48289, 51438, 54720,
     58139, 61698, 65399, 69245, 73239},
    {22785, 24737, 26797, 28969, 31255,
     33659, 36182, 38829, 41601, 44502,
     47536, 50704, 54010, 57456, 61046,
     64783, 68669, 72707, 76901, 81252},
    {25915, 28074, 30349, 32744, 35261,
     37905, 40678, 43582, 46622, 49799,
     53118, 56581, 60192, 63953, 67867,
     71938, 76169, 80562, 85122, 89850},
    {29350, 31728, 34232, 36864, 39628,
     42527, 45563, 48741, 52063, 55533,
     59153, 62928, 66860, 70952, 75208,
     79631, 84224, 88991, 93934, 99056},
    {33108, 35720, 38467, 41351, 44376,
     47544, 50860, 54327, 57947, 61725,
     65664, 69767, 74037, 78478, 83094,
     87886, 92860, 98018, 103363, 108899},
    {37209, 40070, 43074, 46225, 49525,
     52979, 56590, 60362, 64297, 68400,
     72674, 77122, 81748, 86556, 91548,
     96729, 102102, 107670, 113437, 119406},
    {41673, 44797, 48074, 51506, 55098,
     58854, 62776, 66869, 71136, 75581,
     80207, 85018, 90018, 95210, 100598,
     106186, 111977, 117974, 124182, 130604},
    {46520, 49923, 53487, 57218, 61117,
     65191, 69441, 73872, 78488, 83292,
     88288, 93480, 98872, 104468, 110270,
     116283, 122512, 128958, 135627, 142522},
    {51772, 55468, 59337, 63381, 67605,
     72013, 76608, 81395, 86377, 91558,
     96943, 102534, 108337, 114354, 120590,
     127049, 133735, 140651, 147801, 155190},
    {57449, 61456, 65645, 70020, 74585,
     79344, 84302, 89462, 94828, 100405,
     106196, 112206, 118438, 124897, 131587,
     138511, 145674, 153080, 160732, 168636},
    {63575, 67908, 72434, 77157, 82080,
     87209, 92547, 98098, 103867, 109858,
     116075, 122522, 129204, 136124, 143287,
     150697, 158358, 166275, 174451, 182891},
    {70172, 74849, 79729, 84816, 90116,
     95632, 101368, 107329, 113520, 119944,
     126606, 133511, 140662, 148063, 155720,
     163637, 171817, 180266, 188987, 197985},
    {77263, 82301, 87552, 93023, 98717,
     104638, 110792, 117182, 123813, 130690,
     137818, 145199, 152840, 160744, 168916,
     177360, 186081, 195083, 204372, 213950},
    {84872, 90288, 95930, 101801, 107908,
     114254, 120844, 127683, 134775, 142124,
     149737, 157616, 165767, 174194, 182902,
     191896, 201180, 210758, 220636, 230818},
    {93024, 98837, 104886, 111178, 117716,
     124506, 131552, 138859, 146431, 154274,
     162392, 170790, 179473, 188445, 197711,
     207276, 217145, 227322, 237812, 248620},
    {101744, 107971, 114448, 121178, 128168,
     135421, 142943, 150738, 158812, 167168,
     175813, 184751, 193987, 203526, 213373,
     223531, 234008, 244807, 255933, 267391},
    {111056, 117718, 124641, 131830, 139290,
     147027, 155045, 163349, 171945, 180837,
     190030, 199530, 209341, 219469, 229918,
     240694, 251801, 263245, 275030, 287162},
    {120988, 128103, 135492, 143159, 151111,
     159352, 167887, 176721, 185860, 195309,
     205072, 215156, 225565, 236305, 247380,
     258796, 270557, 282670, 295139, 307969},
    {131565, 139154, 147029, 155195, 163658,
     172424, 181497, 190883, 200587, 210615,
     220971, 231662, 242691, 254066, 265790,
     277870, 290310, 303116, 316293, 329847},
    {142816, 150898, 159279, 167965, 176962,
     186273, 195906, 205866, 216157, 226786,
     237758, 249078, 260752, 272785, 285182,
     297950, 311093, 324616, 338527, 352829},
    {154767, 163363, 172272, 181499, 191050,
     200930, 211145, 221700, 232601, 243854,
     255465, 267438, 279779, 292494, 305589,
     319069, 332940, 347207, 361876, 376952},
	/* 41 level */
    {267447, 276579, 286037, 295826, 305953,
     316423, 327242, 338416, 349951, 361851,
     374124, 386774, 399807, 413229, 427046,
     441263, 455887, 470923, 486376, 402252},
	/* 42 level */
    {495112, 505379, 516000, 526981, 538328,
     660047, 692144, 734625, 767496, 800763,
     854432, 878509, 893000, 917911, 953248,
     1019017, 1075224, 1101875, 1108976, 1206533},
	/* 43 level */
    {695112, 705379, 816000, 926981, 998328,
     1050047, 1092144, 1174625, 1287496, 1300763,
     1394432, 1428509, 1543000, 1617911, 1773248,
     1889017, 1905224, 1921875, 1938976, 1956533}
};

/* AC data for mobile balancing */
int mob_bal_ac[43][20] =
{
    {100, 100, 100, 100, 100,
     100, 100, 100, 100, 100,
     100, 100, 100, 100, 100,
     100, 100, 99, 99, 99},
    {100, 100, 100, 100, 100,
     100, 100, 100, 100, 100,
     99, 99, 99, 99, 99,
     99, 98, 98, 98, 98},
    {100, 100, 100, 100, 100,
     100, 100, 99, 99, 99,
     99, 98, 98, 98, 98,
     97, 97, 97, 96, 96},
    {100, 100, 100, 100, 100,
     99, 99, 99, 98, 98,
     98, 97, 97, 97, 96,
     96, 95, 95, 94, 94},
    {100, 100, 100, 99, 99,
     99, 98, 98, 98, 97,
     97, 96, 96, 95, 94,
     94, 93, 93, 92, 91},
    {100, 99, 99, 99, 98,
     98, 97, 97, 96, 96,
     95, 95, 94, 93, 93,
     92, 91, 90, 89, 88},
    {99, 99, 98, 98, 97,
     97, 96, 96, 95, 94,
     94, 93, 92, 91, 90,
     89, 88, 87, 86, 85},
    {99, 98, 98, 97, 96,
     96, 95, 94, 94, 93,
     92, 91, 90, 89, 88,
     87, 85, 84, 83, 82},
    {98, 97, 97, 96, 95,
     94, 94, 93, 92, 91,
     90, 89, 87, 86, 85,
     84, 82, 81, 79, 78},
    {97, 96, 95, 95, 94,
     93, 92, 91, 90, 88,
     87, 86, 85, 83, 82,
     80, 79, 77, 75, 73},
    {96, 95, 94, 93, 92,
     91, 90, 89, 87, 86,
     85, 83, 81, 80, 78,
     76, 75, 73, 71, 69},
    {94, 93, 92, 91, 90,
     89, 88, 86, 85, 83,
     81, 80, 78, 76, 74,
     72, 70, 68, 66, 64},
    {93, 92, 91, 89, 88,
     86, 85, 83, 82, 80,
     78, 76, 74, 72, 70,
     68, 65, 63, 61, 58},
    {91, 90, 88, 87, 85,
     84, 82, 80, 78, 76,
     74, 72, 70, 68, 65,
     63, 60, 57, 55, 52},
    {89, 87, 86, 84, 82,
     81, 79, 77, 75, 72,
     70, 68, 65, 63, 60,
     57, 54, 51, 48, 45},
    {87, 85, 83, 81, 79,
     77, 75, 73, 70, 68,
     66, 63, 60, 57, 54,
     51, 48, 45, 42, 38},
    {84, 82, 80, 78, 76,
     74, 71, 69, 66, 63,
     61, 58, 55, 51, 48,
     45, 42, 38, 34, 31},
    {81, 79, 77, 74, 72,
     69, 67, 64, 61, 58,
     55, 52, 49, 45, 42,
     38, 34, 31, 27, 23},
    {78, 75, 73, 70, 68,
     65, 62, 59, 56, 53,
     49, 46, 42, 38, 35,
     31, 27, 22, 18, 14},
    {74, 71, 69, 66, 63,
     60, 57, 53, 50, 46,
     43, 39, 35, 31, 27,
     23, 18, 14, 9, 4},
    {70, 67, 64, 61, 58,
     55, 51, 48, 44, 40,
     36, 32, 28, 23, 19,
     14, 10, 5, 0, -5},
    {66, 62, 59, 56, 52,
     49, 45, 41, 37, 33,
     29, 24, 20, 15, 10,
     5, 0, -5, -10, -16},
    {61, 57, 54, 50, 46,
     42, 38, 34, 30, 25,
     21, 16, 11, 6, 1,
     -4, -10, -15, -21, -27},
    {55, 52, 48, 44, 40,
     36, 31, 27, 22, 17,
     12, 7, 2, -3, -9,
     -15, -21, -27, -33, -39},
    {50, 46, 42, 37, 33,
     28, 24, 19, 14, 9,
     3, -2, -8, -14, -20,
     -26, -32, -38, -45, -51},
    {44, 39, 35, 30, 26,
     21, 16, 10, 5, -1,
     -6, -12, -18, -24, -31,
     -37, -44, -51, -57, -65},
    {37, 32, 28, 23, 18,
     12, 7, 1, -4, -10,
     -16, -23, -29, -36, -42,
     -49, -56, -64, -71, -78},
    {30, 25, 20, 14, 9,
     3, -2, -8, -14, -21,
     -27, -34, -41, -48, -55,
     -62, -70, -77, -85, -93},
    {22, 17, 11, 6, 0,
     -6, -12, -19, -25, -32,
     -39, -46, -53, -60, -68,
     -76, -84, -92, -100, -108},
    {14, 8, 2, -4, -10,
     -16, -23, -29, -36, -44,
     -51, -58, -66, -74, -82,
     -90, -98, -107, -116, -125},
    {5, -1, -7, -13, -20,
     -27, -34, -41, -48, -56,
     -64, -71, -80, -88, -96,
     -105, -114, -123, -132, -141},
    {-4, -10, -17, -24, -31,
     -38, -46, -53, -61, -69,
     -77, -85, -94, -103, -112,
     -121, -130, -140, -149, -159},
    {-14, -21, -28, -35, -42,
     -50, -58, -66, -74, -83,
     -91, -100, -109, -118, -128,
     -137, -147, -157, -167, -178},
    {-24, -32, -39, -47, -55,
     -63, -71, -79, -88, -97,
     -106, -115, -125, -135, -144,
     -155, -165, -175, -186, -197},
    {-36, -43, -51, -59, -68,
     -76, -85, -94, -103, -112,
     -122, -131, -141, -152, -162,
     -173, -183, -194, -206, -217},
    {-47, -55, -64, -72, -81,
     -90, -99, -109, -118, -128,
     -138, -148, -159, -170, -180,
     -192, -203, -214, -226, -238},
    {-60, -68, -77, -86, -95,
     -105, -114, -124, -134, -145,
     -155, -166, -177, -188, -200,
     -211, -223, -235, -248, -260},
    {-73, -82, -91, -101, -110,
     -120, -130, -141, -151, -162,
     -173, -185, -196, -208, -220,
     -232, -244, -257, -270, -283},
    {-87, -96, -106, -116, -126,
     -136, -147, -158, -169, -180,
     -192, -204, -216, -228, -241,
     -253, -266, -280, -293, -307},
    {-101, -111, -121, -132, -143,
     -153, -165, -176, -188, -200,
     -212, -224, -237, -249, -263,
     -276, -289, -303, -317, -332},
    {-116, -127, -138, -149, -160,
     -171, -183, -195, -207, -219,
     -232, -245, -258, -272, -285,
     -299, -313, -328, -342, -357},
    {-132, -143, -155, -166, -178,
     -190, -202, -215, -227, -240,
     -253, -267, -281, -295, -309,
     -323, -338, -353, -368, -384},
    // NOTE: Original (legacy 2003)
    {-149, -161, -172, -184, -197,
     -209, -222, -235, -248, -262,
     -276, -290, -304, -319, -333,
     -349, -364, -380, -395, -412}
/*
    // NOTE: New (Legacy 2010)
    { -249, -261, -272, -284, -297,
      -309, -322, -335, -348, -362,
      -376, -390, -404, -419, -433,
      -449, -564, -680, -695, -712}
*/
};


/* HR data for mobile balancing */
int mob_bal_hr[43][20] =
{
    {0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0,
     0, 1, 1, 1, 1,
     1, 1, 1, 2, 2,
     2, 2, 2, 2, 3},
    {0, 0, 1, 1, 1,
     1, 1, 1, 2, 2,
     2, 2, 2, 3, 3,
     3, 3, 4, 4, 4},
    {1, 1, 1, 1, 1,
     2, 2, 2, 2, 3,
     3, 3, 4, 4, 4,
     5, 5, 5, 6, 6},
    {1, 1, 1, 2, 2,
     2, 3, 3, 3, 4,
     4, 4, 5, 5, 6,
     6, 7, 7, 8, 8},
    {1, 2, 2, 2, 3,
     3, 3, 4, 4, 5,
     5, 6, 6, 7, 7,
     8, 8, 9, 10, 10},
    {2, 2, 3, 3, 3,
     4, 4, 5, 5, 6,
     6, 7, 8, 8, 9,
     9, 10, 11, 12, 12},
    {2, 3, 3, 4, 4,
     5, 5, 6, 6, 7,
     8, 8, 9, 10, 11,
     11, 12, 13, 14, 15},
    {3, 4, 4, 5, 5,
     6, 6, 7, 8, 8,
     9, 10, 11, 11, 12,
     13, 14, 15, 16, 17},
    {4, 4, 5, 5, 6,
     7, 7, 8, 9, 10,
     11, 11, 12, 13, 14,
     15, 16, 17, 18, 20},
    {4, 5, 6, 6, 7,
     8, 9, 9, 10, 11,
     12, 13, 14, 15, 16,
     17, 19, 20, 21, 22},
    {5, 6, 7, 7, 8,
     9, 10, 11, 12, 13,
     14, 15, 16, 17, 18,
     20, 21, 22, 24, 25},
    {6, 7, 8, 8, 9,
     10, 11, 12, 13, 14,
     16, 17, 18, 19, 21,
     22, 23, 25, 26, 28},
    {7, 8, 9, 10, 11,
     12, 13, 14, 15, 16,
     18, 19, 20, 22, 23,
     25, 26, 28, 29, 31},
    {8, 9, 10, 11, 12,
     13, 14, 15, 17, 18,
     19, 21, 22, 24, 25,
     27, 29, 31, 32, 34},
    {9, 10, 11, 12, 13,
     15, 16, 17, 19, 20,
     22, 23, 25, 26, 28,
     30, 32, 34, 35, 37},
    {10, 11, 12, 14, 15,
     16, 18, 19, 21, 22,
     24, 25, 27, 29, 31,
     33, 35, 37, 39, 41},
    {11, 13, 14, 15, 17,
     18, 19, 21, 23, 24,
     26, 28, 30, 32, 34,
     36, 38, 40, 42, 44},
    {13, 14, 15, 17, 18,
     20, 21, 23, 25, 27,
     28, 30, 32, 34, 37,
     39, 41, 43, 46, 48},
    {14, 15, 17, 18, 20,
     22, 23, 25, 27, 29,
     31, 33, 35, 37, 40,
     42, 44, 47, 49, 52},
    {15, 17, 19, 20, 22,
     24, 26, 27, 29, 32,
     34, 36, 38, 40, 43,
     45, 48, 51, 53, 56},
    {17, 19, 20, 22, 24,
     26, 28, 30, 32, 34,
     36, 39, 41, 44, 46,
     49, 52, 54, 57, 60},
    {19, 20, 22, 24, 26,
     28, 30, 32, 35, 37,
     39, 42, 44, 47, 50,
     53, 55, 58, 61, 64},
    {20, 22, 24, 26, 28,
     30, 33, 35, 37, 40,
     42, 45, 48, 51, 53,
     56, 59, 62, 66, 69},
    {22, 24, 26, 28, 31,
     33, 35, 38, 40, 43,
     46, 48, 51, 54, 57,
     60, 63, 67, 70, 74},
    {24, 26, 28, 31, 33,
     35, 38, 40, 43, 46,
     49, 52, 55, 58, 61,
     64, 68, 71, 75, 78},
    {26, 28, 31, 33, 35,
     38, 41, 43, 46, 49,
     52, 55, 58, 62, 65,
     69, 72, 76, 79, 83},
    {28, 31, 33, 36, 38,
     41, 44, 47, 50, 53,
     56, 59, 62, 66, 69,
     73, 77, 81, 84, 88},
    {30, 33, 35, 38, 41,
     44, 47, 50, 53, 56,
     59, 63, 66, 70, 74,
     78, 81, 85, 89, 94},
    {33, 35, 38, 41, 44,
     47, 50, 53, 56, 60,
     63, 67, 71, 74, 78,
     82, 86, 90, 95, 99},
    {35, 38, 41, 44, 47,
     50, 53, 57, 60, 64,
     67, 71, 75, 79, 83,
     87, 91, 96, 100, 105},
    {38, 41, 44, 47, 50,
     53, 57, 60, 64, 68,
     71, 75, 79, 83, 88,
     92, 97, 101, 106, 111},
    {40, 43, 47, 50, 53,
     57, 60, 64, 68, 72,
     76, 80, 84, 88, 93,
     97, 102, 107, 112, 117},
    {43, 46, 50, 53, 57,
     60, 64, 68, 72, 76,
     80, 84, 89, 93, 98,
     103, 107, 112, 117, 123},
    {46, 49, 53, 56, 60,
     64, 68, 72, 76, 80,
     85, 89, 94, 98, 103,
     108, 113, 118, 124, 129},
    {49, 53, 56, 60, 64,
     68, 72, 76, 80, 85,
     89, 94, 99, 104, 109,
     114, 119, 124, 130, 136},
    {52, 56, 60, 64, 68,
     72, 76, 80, 85, 89,
     94, 99, 104, 109, 114,
     120, 125, 131, 136, 142},
    {56, 59, 63, 67, 72,
     76, 80, 85, 89, 94,
     99, 104, 109, 115, 120,
     126, 131, 137, 143, 149},
    {59, 63, 67, 71, 76,
     80, 85, 89, 94, 99,
     104, 110, 115, 121, 126,
     132, 138, 144, 150, 156},
    {63, 67, 71, 75, 80,
     85, 89, 94, 99, 104,
     110, 115, 121, 127, 132,
     138, 144, 151, 157, 164},
    {266, 271, 275, 280, 284,
     289, 294, 299, 304, 310,
     315, 321, 327, 333, 339,
     345, 351, 358, 364, 371},
    {270, 275, 279, 284, 289,
     294, 299, 304, 310, 315,
     321, 327, 333, 339, 345,
     352, 358, 365, 372, 379},
    {274, 279, 283, 288, 293,
     299, 304, 310, 315, 321,
     327, 333, 339, 345, 352,
     359, 365, 372, 379, 387}
};

/* DR data for mobile balancing */
int mob_bal_dr[43][20] =
{
    {0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1,
     1, 1, 1, 1, 1,
     1, 1, 1, 2, 2,
     2, 2, 2, 2, 2},
    {1, 1, 1, 1, 2,
     2, 2, 2, 2, 2,
     2, 2, 3, 3, 3,
     3, 3, 3, 4, 4},
    {2, 2, 2, 2, 2,
     3, 3, 3, 3, 3,
     3, 4, 4, 4, 4,
     5, 5, 5, 5, 5},
    {2, 3, 3, 3, 3,
     4, 4, 4, 4, 4,
     5, 5, 5, 6, 6,
     6, 6, 7, 7, 7},
    {3, 4, 4, 4, 4,
     5, 5, 5, 5, 6,
     6, 6, 7, 7, 7,
     8, 8, 8, 9, 9},
    {4, 5, 5, 5, 5,
     6, 6, 6, 7, 7,
     8, 8, 8, 9, 9,
     9, 10, 10, 11, 11},
    {5, 6, 6, 6, 7,
     7, 7, 8, 8, 9,
     9, 10, 10, 10, 11,
     11, 12, 12, 13, 13},
    {6, 7, 7, 8, 8,
     8, 9, 9, 10, 10,
     11, 11, 12, 12, 13,
     13, 14, 15, 15, 16},
    {8, 8, 8, 9, 9,
     10, 10, 11, 12, 12,
     13, 13, 14, 14, 15,
     16, 16, 17, 18, 18},
    {9, 9, 10, 10, 11,
     12, 12, 13, 13, 14,
     15, 15, 16, 17, 17,
     18, 19, 19, 20, 21},
    {10, 11, 11, 12, 13,
     13, 14, 15, 15, 16,
     17, 17, 18, 19, 20,
     20, 21, 22, 23, 24},
    {12, 12, 13, 14, 14,
     15, 16, 17, 17, 18,
     19, 20, 20, 21, 22,
     23, 24, 25, 26, 27},
    {13, 14, 15, 16, 16,
     17, 18, 19, 19, 20,
     21, 22, 23, 24, 25,
     26, 27, 28, 29, 30},
    {15, 16, 17, 18, 18,
     19, 20, 21, 22, 23,
     24, 25, 26, 27, 28,
     29, 30, 31, 32, 33},
    {17, 18, 19, 20, 20,
     21, 22, 23, 24, 25,
     26, 27, 28, 30, 31,
     32, 33, 34, 35, 36},
    {19, 20, 21, 22, 23,
     24, 25, 26, 27, 28,
     29, 30, 31, 33, 34,
     35, 36, 37, 39, 40},
    {21, 22, 23, 24, 25,
     26, 27, 29, 30, 31,
     32, 33, 34, 36, 37,
     38, 40, 41, 42, 44},
    {23, 24, 26, 27, 28,
     29, 30, 31, 33, 34,
     35, 36, 38, 39, 41,
     42, 43, 45, 46, 48},
    {26, 27, 28, 29, 31,
     32, 33, 34, 36, 37,
     38, 40, 41, 43, 44,
     46, 47, 49, 50, 52},
    {28, 29, 31, 32, 33,
     35, 36, 37, 39, 40,
     42, 43, 45, 46, 48,
     50, 51, 53, 55, 56},
    {31, 32, 34, 35, 36,
     38, 39, 41, 42, 44,
     45, 47, 49, 50, 52,
     54, 55, 57, 59, 61},
    {34, 35, 37, 38, 40,
     41, 43, 44, 46, 47,
     49, 51, 53, 54, 56,
     58, 60, 62, 64, 65},
    {37, 38, 40, 41, 43,
     45, 46, 48, 50, 51,
     53, 55, 57, 59, 60,
     62, 64, 66, 68, 70},
    {40, 41, 43, 45, 46,
     48, 50, 52, 53, 55,
     57, 59, 61, 63, 65,
     67, 69, 71, 73, 75},
    {43, 45, 47, 48, 50,
     52, 54, 56, 58, 59,
     61, 63, 66, 68, 70,
     72, 74, 76, 78, 81},
    {47, 48, 50, 52, 54,
     56, 58, 60, 62, 64,
     66, 68, 70, 72, 75,
     77, 79, 82, 84, 86},
    {50, 52, 54, 56, 58,
     60, 62, 64, 66, 68,
     71, 73, 75, 77, 80,
     82, 85, 87, 89, 92},
    {54, 56, 58, 60, 62,
     64, 66, 69, 71, 73,
     75, 78, 80, 83, 85,
     88, 90, 93, 95, 98},
    {58, 60, 62, 64, 66,
     69, 71, 73, 76, 78,
     81, 83, 86, 88, 91,
     93, 96, 99, 101, 104},
    {62, 64, 66, 69, 71,
     73, 76, 78, 81, 83,
     86, 88, 91, 94, 96,
     99, 102, 105, 108, 111},
    {66, 69, 71, 73, 76,
     78, 81, 83, 86, 89,
     91, 94, 97, 100, 102,
     105, 108, 111, 114, 117},
    {71, 73, 76, 78, 81,
     83, 86, 89, 91, 94,
     97, 100, 103, 106, 109,
     112, 115, 118, 121, 124},
    {75, 78, 81, 83, 86,
     89, 91, 94, 97, 100,
     103, 106, 109, 112, 115,
     118, 121, 124, 128, 131},
    {80, 83, 86, 88, 91,
     94, 97, 100, 103, 106,
     109, 112, 115, 118, 122,
     125, 128, 132, 135, 138},
    {85, 88, 91, 94, 97,
     100, 103, 106, 109, 112,
     115, 118, 122, 125, 128,
     132, 135, 139, 142, 146},
    {91, 93, 96, 99, 102,
     106, 109, 112, 115, 118,
     122, 125, 129, 132, 136,
     139, 143, 146, 150, 154},
    {96, 99, 102, 105, 108,
     112, 115, 118, 122, 125,
     129, 132, 136, 139, 143,
     147, 150, 154, 158, 162},
    {102, 105, 108, 111, 115,
     118, 121, 125, 128, 132,
     136, 139, 143, 147, 150,
     154, 158, 162, 166, 170},
/* 40 level */
    {108, 111, 114, 118, 121,
     125, 128, 132, 135, 139,
     143, 147, 150, 154, 158,
     162, 166, 170, 174, 179},
/*
   { 526, 542, 558, 575, 591,
   608, 626, 643, 661, 679,
   697, 716, 734, 753, 773,
   792, 812, 832, 852, 872 },
 */
/* stronger */
    {555, 572, 589, 606, 624,
     641, 659, 678, 696, 715,
     734, 753, 772, 792, 812,
     832, 852, 873, 894, 915},
    {586, 603, 621, 639, 657,
     675, 694, 713, 732, 752,
     771, 791, 811, 832, 852,
     873, 894, 916, 938, 960},
    {618, 636, 654, 673, 692,
     711, 730, 750, 769, 790,
     810, 831, 852, 873, 894,
     916, 938, 960, 982, 1005}
/* weak */
/*
   { 114, 117, 121, 124, 128,
   131, 135, 139, 142, 146,
   150, 154, 158, 162, 166,
   170, 175, 179, 183, 188 },
   { 120, 123, 127, 131, 134,
   138, 142, 146, 150, 154,
   158, 162, 166, 170, 175,
   179, 183, 188, 192, 197 },
   { 126, 130, 134, 138, 142,
   145, 149, 153, 158, 162,
   166, 170, 174, 179, 183,
   188, 192, 197, 201, 206}
 */
};

/* EXP data for mobile balancing */
int mob_bal_exp[43][20] =
{
    {54, 55, 55, 55, 56,
     56, 57, 57, 58, 59,
     59, 60, 61, 61, 62,
     63, 64, 64, 65, 66},
    {140, 142, 146, 150, 154,
     158, 162, 168, 172, 176,
     182, 188, 194, 200, 206,
     212, 218, 224, 232, 240},
    {297, 309, 321, 336, 348,
     363, 378, 396, 411, 429,
     447, 465, 486, 507, 528,
     549, 573, 597, 624, 648},
    {576, 608, 640, 672, 708,
     744, 780, 820, 864, 904,
     952, 1000, 1048, 1096, 1152,
     1204, 1264, 1320, 1384, 1448},
    {1050, 1110, 1175, 1240, 1310,
     1385, 1465, 1545, 1630, 1720,
     1810, 1905, 2005, 2110, 2215,
     2325, 2445, 2560, 2685, 2815},
    {1788, 1896, 2016, 2136, 2268,
     2400, 2538, 2688, 2838, 3000,
     3162, 3336, 3510, 3696, 3888,
     4092, 4296, 4512, 4734, 4962},
    {2891, 3073, 3269, 3472, 3682,
     3906, 4137, 4382, 4634, 4893,
     5166, 5453, 5747, 6055, 6370,
     6699, 7042, 7392, 7756, 8134},
    {4464, 4760, 5064, 5376, 5712,
     6056, 6416, 6800, 7184, 7592,
     8016, 8456, 8912, 9384, 9880,
     10384, 10912, 11456, 12016, 12600},
    {6651, 7092, 7542, 8010, 8505,
     9018, 9558, 10116, 10692, 11295,
     11925, 12573, 13248, 13950, 14670,
     15417, 16191, 16992, 17820, 18675},
    {9610, 10230, 10880, 11550, 12260,
     13000, 13760, 14560, 15380, 16240,
     17130, 18060, 19010, 20010, 21030,
     22100, 23190, 24330, 25500, 26710},
    {13497, 14366, 15268, 16203, 17182,
     18205, 19261, 20361, 21505, 22693,
     23914, 25190, 26510, 27885, 29293,
     30756, 32263, 33825, 35431, 37092},
    {18540, 19704, 20928, 22200, 23520,
     24888, 26316, 27804, 29340, 30936,
     32592, 34308, 36084, 37920, 39816,
     41772, 43800, 45888, 48048, 50268},
    {24947, 26494, 28106, 29783, 31538,
     33358, 35243, 37193, 39221, 41327,
     43511, 45760, 48100, 50505, 53001,
     55575, 58240, 60983, 63804, 66716},
    {32970, 35000, 37086, 39270, 41538,
     43904, 46340, 48888, 51506, 54236,
     57050, 59962, 62972, 66094, 69300,
     72632, 76048, 79576, 83216, 86968},
    {42930, 45510, 48195, 50985, 53880,
     56895, 60015, 63240, 66600, 70065,
     73650, 77355, 81180, 85140, 89220,
     93435, 97770, 102255, 106860, 111615},
    {55120, 58384, 61776, 65296, 68944,
     72720, 76640, 80704, 84912, 89264,
     93760, 98416, 103216, 108160, 113280,
     118544, 123984, 129584, 135344, 141280},
    {69921, 73984, 78200, 82586, 87125,
     91834, 96696, 101745, 106964, 112353,
     117929, 123692, 129625, 135762, 142086,
     148614, 155329, 162231, 169354, 176681},
    {87714, 92718, 97920, 103302, 108900,
     114678, 120672, 126864, 133272, 139878,
     146718, 153774, 161046, 168552, 176292,
     184266, 192474, 200934, 209628, 218574},
    {108946, 115064, 121391, 127965, 134767,
     141816, 149093, 156636, 164407, 172444,
     180747, 189297, 198132, 207233, 216600,
     226271, 236208, 246430, 256956, 267767},
    {134100, 141480, 149140, 157080, 165300,
     173800, 182580, 191660, 201020, 210700,
     220700, 230980, 241600, 252540, 263800,
     275400, 287320, 299580, 312200, 325160},
    {163674, 172557, 181734, 191247, 201075,
     211239, 221760, 232617, 243810, 255360,
     267288, 279573, 292236, 305277, 318696,
     332493, 346710, 361305, 376320, 391734},
    {198286, 208846, 219780, 231088, 242770,
     254870, 267344, 280214, 293502, 307208,
     321332, 335896, 350878, 366322, 382184,
     398530, 415316, 432564, 450296, 468512},
    {238533, 251022, 263948, 277311, 291111,
     305371, 320091, 335271, 350934, 367080,
     383709, 400844, 418462, 436609, 455262,
     474444, 494178, 514441, 535233, 556600},
    {285096, 299784, 314976, 330648, 346848,
     363576, 380832, 398616, 416952, 435864,
     455328, 475344, 495960, 517176, 538968,
     561360, 584400, 608040, 632304, 657216},
    {338750, 355900, 373625, 391925, 410850,
     430350, 450450, 471175, 492550, 514525,
     537175, 560475, 584450, 609075, 634400,
     660425, 687150, 714575, 742725, 771600},
    {400218, 420160, 440778, 462020, 483964,
     506584, 529906, 553930, 578682, 604136,
     630344, 657306, 685022, 713518, 742768,
     772824, 803686, 835354, 867854, 901186},
    {470448, 493506, 517320, 541890, 567216,
     593325, 620244, 647946, 676458, 705807,
     735993, 767043, 798930, 831708, 865377,
     899937, 935388, 971784, 1009098, 1047384},
    {550284, 576856, 604240, 632492, 661612,
     691628, 722540, 754348, 787080, 820736,
     855344, 890932, 927500, 965020, 1003576,
     1043112, 1083712, 1125320, 1167992, 1211756},
    {640784, 671205, 702612, 734976, 768297,
     802633, 837984, 874350, 911760, 950214,
     989741, 1030341, 1072043, 1114876, 1158811,
     1203906, 1250161, 1297576, 1346180, 1395973},
    {742980, 777720, 813540, 850440, 888450,
     927600, 967830, 1009260, 1051830, 1095600,
     1140540, 1186710, 1234110, 1282770, 1332690,
     1383900, 1436370, 1490190, 1545300, 1601790},
    {858018, 897543, 938246, 980189, 1023372,
     1067795, 1113489, 1160454, 1208752, 1258352,
     1309285, 1361582, 1415274, 1470361, 1526843,
     1584751, 1644116, 1704938, 1767248, 1831077},
    {987136, 1031904, 1078048, 1125536, 1174400,
     1224640, 1276320, 1329408, 1384000, 1440032,
     1497536, 1556608, 1617184, 1679296, 1743008,
     1808320, 1875200, 1943744, 2013952, 2085824},
    {1131603, 1182192, 1234266, 1287858, 1342968,
     1399629, 1457874, 1517736, 1579182, 1642278,
     1707024, 1773453, 1841598, 1911459, 1983069,
     2056461, 2131635, 2208624, 2287428, 2368080},
    {1292850, 1349800, 1408416, 1468698, 1530680,
     1594362, 1659812, 1727030, 1796050, 1866872,
     1939530, 2014058, 2090456, 2168792, 2249032,
     2331244, 2415428, 2501652, 2589848, 2680152},
    {1472345, 1536290, 1602020, 1669640, 1739115,
     1810515, 1883840, 1959125, 2036370, 2115645,
     2196950, 2280285, 2365720, 2453255, 2542960,
     2634800, 2728810, 2825060, 2923550, 3024280},
    {1671660, 1743192, 1816776, 1892412, 1970100,
     2049876, 2131776, 2215836, 2302092, 2390580,
     2481264, 2574252, 2669508, 2767104, 2867040,
     2969352, 3074076, 3181248, 3290868, 3403008},
    {1892439, 1972322, 2054425, 2138748, 2225365,
     2314313, 2405592, 2499239, 2595254, 2693748,
     2794684, 2898099, 3004067, 3112551, 3223662,
     3337363, 3453691, 3572720, 3694450, 3818881},
    {2136512, 2225432, 2316784, 2410644, 2506974,
     2605850, 2707272, 2811316, 2918020, 3027346,
     3139408, 3254168, 3371702, 3492048, 3615206,
     3741252, 3870186, 4002046, 4136870, 4274696},
    {2405676, 2504463, 2605902, 2710032, 2816931,
     2926599, 3039075, 3154398, 3272607, 3393702,
     3517800, 3644862, 3774966, 3908112, 4044339,
     4183725, 4326270, 4472052, 4621032, 4773288},
    {2702000, 2811440, 2923800, 3039120, 3157440,
     3278800, 3403200, 3530720, 3661400, 3795280,
     3932400, 4072720, 4216400, 4363400, 4513800,
     4667600, 4824840, 4985600, 5149880, 5317720},
    {3027481, 3148513, 3272702, 3400089, 3530797,
     3664785, 3802094, 3942847, 4087003, 4234644,
     4385811, 4540504, 4698805, 4860755, 5026354,
     5195725, 5368827, 5545742, 5726511, 5911134},
    {3384444, 3517920, 3654924, 3795372, 3939432,
     4087062, 4238346, 4393284, 4552002, 4714500,
     4880778, 5050962, 5225052, 5403090, 5585118,
     5771220, 5961396, 6155730, 6354180, 6556914},
    {3775142, 3922116, 4072874, 4227459, 4385871,
     4548239, 4714520, 4884843, 5059208, 5237701,
     5420322, 5607114, 5798206, 5993555, 6193247,
     6397325, 6605832, 6818854, 7036391, 7258529}
};
