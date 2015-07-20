//
//  analysis.cpp
//  FireflyProject
//
//  Created by Keisuke Karijuku on 2013/12/11.
//  Copyright (c) 2013年 Keisuke Karijuku. All rights reserved.
//

#include "analysis.h"
#include<vector>
#include<iostream>

/*
 &flucP		周期揺らぎ
 sn			音声データ
 fsize		零点修正誤差範囲（秒）
 dsize		相関関数の幅（秒）
 dmin		求める周期幅の最小（秒）
 dmax		求める周期幅の最大（秒）
 sampPerSec	サンプリング周波数（Hz）
 */

void GetFlucPeriod( std::vector<int>& flucP, const std::vector<double> sn,const double fsize,
                   const double dsize, const double dmin, const double dmax,const unsigned int sampPerSec)
{
	std::cout << "DFF now in progress..." << std::endl;
    
	flucP.clear();
    
	std::vector<double> temp_x,temp_y,r;	// 相関関数
	std::vector<int> index,fixindex;		// index:相関関数で求まる周期（index），fixindex:零点に修正した周期(index)
	int offset;
	int pmin,pmax,p;
	int fixsize;
	int n,m;
	double max;
	unsigned int count=0,max_count=0,miss_count=0;	// 零点検出の連続回数
	unsigned int zero_min_i=0,zero_max_i=0;
    
	offset = 0;
    
	temp_x.resize( (int)(sampPerSec * dsize) );
	temp_y.resize( temp_x.size() );
    
	fixsize = (int)(sampPerSec * fsize);
	pmin = (int)(sampPerSec * dmin);
	pmax = (int)(sampPerSec * dmax);
    
	r.resize(pmax+1);
    
	while( offset + pmax * 2 < sn.size() )
	{
		for(n=0;n<temp_x.size();n++)
		{
			temp_x[n] = sn[offset + n];
		}
		max = 0.0;
		p=pmin;
		// 相関関数
		for(m=pmin;m<=pmax;m++)
		{
			for(n=0;n<temp_x.size();n++)
			{
				temp_y[n] = sn[offset+m+n];
			}
			r[m]=0.0;
			for(n=0;n<temp_x.size();n++)
			{
				r[m]+=temp_x[n]*temp_y[n];
			}
			if(r[m]>max)
			{
				max=r[m];
				p=m;
			}
		}
		index.push_back(offset);
		offset += p;
	}
	offset = 0.0;
	// 零点修正
	for( n=0;n<index.size();n++)
	{
		for(m=0;m<fixsize;m++)
		{
			if(index[n]+m+1 < sn.size())
			{
				if(sn[index[n]+m] * sn[index[n]+m+1] <= 0.0)
				{
					fixindex.push_back(index[n]+m);
					count++;
					break;
				}
			}
			if(index[n]-m-1 > 0)
			{
				if(sn[index[n]-m] * sn[index[n]-m-1] <= 0.0)
				{
					fixindex.push_back(index[n]-m-1);
					count++;
					break;
				}
			}
			if(m==fixsize-1)
			{
				count = 0;
				miss_count++;
				break;
			}
		}
		if(max_count < count)
		{
			max_count = count;
			zero_max_i = n-miss_count;
			zero_min_i = zero_max_i-(max_count-1);
		}
	}
	std::cout << "零点検出率：" << (double)fixindex.size()/(double)index.size() * 100.0 << "%" << std::endl;
	//*
	for( n=0;n<fixindex.size()-1;n++ )
	{
		flucP.push_back(fixindex[n+1]-fixindex[n]);
	}
	//*/
	/* 零点連続部分のみ抽出する場合
     for( n=zero_min_i+1;n<zero_max_i;n++)
     {
     flucP.push_back(fixindex[n+1]-fixindex[n]);
     }
     //*/
	std::cout << "DFF finish." << std::endl;
}
