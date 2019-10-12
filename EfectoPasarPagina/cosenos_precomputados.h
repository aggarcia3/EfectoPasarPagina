#pragma once

// Inspirado en
// https://cursos.faitic.uvigo.es/tema1718/claroline/document/goto/index.php/2018_Grupos_Reducidos/Enunciados_GR_2018/AP_GR_06_Transiciones_SIMD_-II-/AP_2018_Practica_06.pdf?cidReq=O06G150V01401

// Número entero grande que usaremos para optimizar operaciones aritméticas con cosenos por shifts.
// Debe de ser una potencia de dos
#define G 8192 // 2^13

// Valores enteros precomputados de los cosenos, calculados como
// (int) (G * cos(omegaRad)) para todo omegaRad posible perteneciente a [ 0, 90 ].
// Este array ocupa 4 * 90 = 360 bytes (asumiendo sizeof(int) == 4).
static int cosenosEnteros[91] =
{
    8191,	8190,	8187,	8180,	8172,	8160,	// (Necesitamos el coseno de 0, que es 2^13 - 1)
    8147,	8130,	8112,	8091,	8067,
    8041,	8012,	7982,	7948,	7912,
    7874,	7834,	7791,	7745,	7697,
    7647,	7595,	7540,	7483,	7424,
	7362,	7299,	7233,	7164,	7094,
	7021,	6947,	6870,	6791,	6710,
	6627,	6542,	6455,	6366,	6275,
	6182,	6087,	5991,	5892,	5792,
	5690,	5586,	5481,	5374,	5265,
	5155,	5043,	4930,	4815,	4698,
	4580,	4461,	4341,	4219,	4096,
	3971,	3845,	3719,	3591,	3462,
	3331,	3200,	3068,	2935,	2801,
	2667,	2531,	2395,	2258,	2120,
	1981,	1842,	1703,	1563,	1422,
	1281,	1140,	998,	856,	713,
	571,	428,	285,	142,	0
};

// Valores enteros precomputados de los inversos de los cosenos, calculados como
// (int) (G / cos(omegaRad)) para todo omegaRad posible perteneciente a ( 0, 90 ].
// Este array ocupa 4 * 90 = 360 bytes (asumiendo sizeof(int) == 4).
static int cosenosInversosEnteros[90] =
{
	8193,	8196,	8203,	8212,	8223,
	8237,	8253,	8272,	8294,	8318,
	8345,	8375,	8407,	8442,	8480,
	8522,	8566,	8613,	8664,	8717,
	8774,	8835,	8899,	8967,	9038,
	9114,	9194,	9278,	9366,	9459,
	9557,	9659,	9767,	9881,	10000,
	10125,	10257,	10395,	10541,	10693,
	10854,	11023,	11201,	11388,	11585,
	11792,	12011,	12242,	12486,	12744,
	13017,	13306,	13612,	13937,	14282,
	14649,	15041,	15458,	15905,	16384,
	16897,	17449,	18044,	18687,	19383,
	20140,	20965,	21868,	22859,	23951,
	25162,	26509,	28019,	29720,	31651,
	33862,	36416,	39401,	42932,	47175,
	52366,	58861,	67219,	78370,	93992,
	117437,	156527,	234731,	469390,	524287	// 2^19 - 1, para que no ocurra overflow con operandos hasta 2^13 - 1. En realidad sería +8
};