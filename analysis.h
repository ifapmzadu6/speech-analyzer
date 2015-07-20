//
//  analysis.h
//  FireflyProject
//
//  Created by Keisuke Karijuku on 2013/12/11.
//  Copyright (c) 2013年 Keisuke Karijuku. All rights reserved.
//

#ifndef __FireflyProject__analysis__
#define __FireflyProject__analysis__

#include <iostream>
#include <vector>

// DEFAULT
const double	Dfixsize = 0.001;
const double	Ddsize = 0.004;
const double	Ddmin = 0.006;
const double	Ddmax = 0.0085;
const unsigned int DsampPerSec = 44100;

void GetFlucPeriod( std::vector<int>& flucP, const std::vector<double> sn,
                   const double fixsize = Dfixsize,const double dsize = Ddsize, const double dmin = Ddmin,
                   const double dmax = Ddmax, const unsigned int sampPerSec = DsampPerSec);

#endif /* defined(__FireflyProject__analysis__) */
