/*
 * mywtmatrix.h
 *
 *  Created on: Mar 25, 2014
 *      Author: diegocaro
 */

#ifndef MYWTMATRIX_H_
#define MYWTMATRIX_H_
#include <wavelet_matrix.h>

inline void append_odd(uint symbol, uint freq, uint *res) {
  if (freq % 2 == 1) { *res+=1; res[*res] = symbol; };
}

inline void append_symbol(uint symbol, uint freq, uint *res) {
  *res += 1; res[*res] = symbol;
}

typedef void (*action)(uint, uint, uint*);

class MyWaveletMatrix : public WaveletMatrix {
public:
	void select_all(uint symbol, uint *res);


	template<action F>
	void range_report(uint start, uint end, uint lowvoc, uint uppvoc, uint *res);

protected:
	void _select_all(uint start, uint end, uint symbol, uint level, uint *res);

	template<action F>
	void _range_report(uint start, uint end, uint lowvoc, uint uppvoc, uint vocmin, uint vocmax, uint level,  uint *res);

};


#endif /* MYWTMATRIX_H_ */
