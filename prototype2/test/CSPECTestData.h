/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Dataset for running unit tests - do not edit if unsure of what they
 * do!
 */

#include <vector>

using namespace std;

// clang-format off

vector<unsigned int> ok_one
{
  0x40000009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd
};

vector<unsigned int> ok_two
{
  0x40000009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd
};

vector<unsigned int> ok_16
{
  0x40000009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40020009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40030009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40040009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40050009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40060009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40070009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40080009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x40090009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x400a0009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x400b0009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x400c0009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x400d0009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x400e0009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  0x400f0009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd
};

vector<unsigned int> err_hdr
{
  0x30010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd
};

vector<unsigned int> err_hdr2
{
  0x40010008, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd
};

vector<unsigned int> err_dat
{
  0x40010009, 0x0400020f, 0x8401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd
};

vector<unsigned int> err_ftr
{
  0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0x400211dd
};

vector<unsigned int> err_short
{
  0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c
};

vector<unsigned int> err_long
{
  0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b, 0x040402d3,
  0x04050035, 0x04060385, 0x0407002c, 0xc00211dd, 0x40010009, 0x0400020f,
  0x0401002f, 0x0402028f, 0x0403002b, 0x040402d3, 0x04050035, 0x04060385,
  0x0407002c, 0xc00211dd, 0x40010009
};

vector<unsigned int> err_below_thresh
{ //                   *                          w0 bad g0 ok w1 bad == bad
  0x40000009, 0x0400000f, 0x0401020f, 0x0402028f, 0x0403002b,
  0x040402d3, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,
  //                               *              w0 ok g0 ok w1 ok == ok
  0x40010009, 0x0400020f, 0x0401002f, 0x0402028f, 0x0403002b,
  0x04040203, 0x04050235, 0x04060385, 0x0407002c, 0xc00211dd,

  0x40000009, 0x0400020f, 0x0401020f, 0x0402028f, 0x0403002b,
  //       *                                      w0 ok g0 bad w1 bad == bad
  0x0404000f, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd,

  0x40010009, 0x0400020f, 0x0401022f, 0x0402028f, 0x0403002b,
  //                   *                          w0 ok g0 ok w1 bad == bad
  0x04040203, 0x04050035, 0x04060385, 0x0407002c, 0xc00211dd

};

// clang-format on

/** Raw packet data above, now collect into iterable containers */

vector<vector<unsigned int>> ok{ok_one, ok_two, ok_16};
vector<vector<unsigned int>> err_pkt{err_hdr, err_hdr2, err_dat, err_ftr};
vector<vector<unsigned int>> err_size{err_short, err_long};
