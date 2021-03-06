//
//  BridgeKey.h
//  krypto
//
//  Created by Peng Hui How and Robin Zhang on 7/22/15.
//  Copyright (c) 2015 Kryptnostic. All rights reserved.
//
//  Implementation of bridge key generation
//	Accessed by clients, used to generate components of the public keys (Refer to implementation.pdf for the notations)
//

#ifndef krypto_BridgeKey_h
#define krypto_BridgeKey_h

#include "PrivateKey.h"

/*
 * Template for BridgeKey
 * Plaintext Length: N
 */
template<unsigned int N>
class BridgeKey{
public:

	/*
     * Constructor
     * Constructs a BridgeKey with a given PrivateKey
     * and BitMatrix K (for left-matrix multiplcation)
     */
	BridgeKey(const PrivateKey<N> &pk) :
	_pk(pk),
	_R(BitMatrix<N>::randomInvertibleMatrix()),
	_M(pk.getM()),
	_Cu1(pk.getUnaryObf1()),
	_Cu2(pk.getUnaryObf2()),
	_Cb1(pk.getBinaryObf1()),
	_Cb2(pk.getBinaryObf2())
	{}

/* Unary unified code */

	/*
	 * Function: getUnaryG1
	 * Returns the first function of the obfuscation chain for unary operations (e.g. left matrix multiplication)
	 */
	const MultiQuadTuple<2*N, 2*N> getUnaryG1() const{
		const MultiQuadTupleChain<N,2> & f = _pk.getf();

		const BitMatrix<N, 2*N> & matTop = _M.inv().splitV2(1);
		const BitMatrix<N, 2*N> & matBot = _R * matTop;

		const MultiQuadTuple<2*N, N> & top = f.get(0) * matTop;
		const MultiQuadTuple<2*N, N> & bot = f.get(0) * matBot;
		MultiQuadTuple<2*N, 2*N> aug;// = MultiQuadTuple<2*N, 2*N>::augV(top, bot);
		aug.augV(top, bot);
		return aug.template rMult<2*N>(_Cu1);
	}

	/*
	 * Function: getUnaryG2
	 * Returns the second function of the obfuscation chain for unary operations (e.g. left matrix multiplication)
	 */
	const MultiQuadTuple<2*N, 2*N> getUnaryG2() const{
		const MultiQuadTupleChain<N,2> & f = _pk.getf();

		const BitMatrix<2*N> Cu1i = _Cu1.inv();

		const BitMatrix<N, 2*N> & matTop = Cu1i.splitV2(0);
		const BitMatrix<N, 2*N> & matBot = Cu1i.splitV2(1);

		const MultiQuadTuple<2*N, N> & top = f.get(1) * matTop;
		const MultiQuadTuple<2*N, N> & bot = f.get(1) * matBot;
		MultiQuadTuple<2*N, 2*N> aug;// = MultiQuadTuple<2*N, 2*N>::augV(top, bot);
		aug.augV(top, bot);
		return aug.template rMult<2*N>(_Cu2);
	}

/* Left Matrix Multiplication */

	/*
	 * Function: getLMMZ
	 * Returns matrix Z used for homomorphic left matrix multiplication
	 * Dimension of Z: 2*(N * 2^6) by 4*(N * 2^6)
	 */
	const BitMatrix<2*N, 4*N> getLMMZ(const BitMatrix<N> & K) const{
		const BitMatrix<N> & zeroN = BitMatrix<N>::zeroMatrix();

		const BitMatrix<N, 2*N> & XTop = BitMatrix<N, 2*N>::augH(K, zeroN);
		const BitMatrix<N, 2*N> & XBot = BitMatrix<N, 2*N>::augH(zeroN, _R);
		const BitMatrix<2*N> & X = _M * BitMatrix<2*N>::augV(XTop, XBot) * _M.inv();

		const BitMatrix<N, 2*N> & YTop = BitMatrix<N, 2*N>::augH(K, BitMatrix<N>::identityMatrix());
		const BitMatrix<N, 2*N> & YBot = BitMatrix<N, 2*N>::augH(zeroN, zeroN);
		const BitMatrix<2*N> & Y = _M * BitMatrix<2*N>::augV(YTop, YBot) * _Cu2.inv();
		return BitMatrix<2*N, 4*N>::augH(X, Y);
	}

/* Shift Matrices */
	/*
	 * Function: getLeftShiftMatrix
	 * Returns a matrix LS used for homomorphic left shift
	 * Dimension of LS: 2*(N * 2^6) by 4*(N * 2^6)
	 */
	const BitMatrix<2*N, 4*N> getLeftShiftMatrix() const{
		return getLMMZ(BitMatrix<N>::leftShiftMatrix());
	}	

	/*
	 * Function: getRightShiftMatrix
	 * Returns a matrix RS used for homomorphic right shift
	 * Dimension of RS: 2*(N * 2^6) by 4*(N * 2^6)
	 */
	const BitMatrix<2*N, 4*N> getRightShiftMatrix() const{
		return getLMMZ(BitMatrix<N>::rightShiftMatrix());
	}	

	const BitMatrix<2*N, 4*N> getLeftColumnMatrix() const{
		BitMatrix<N> M = BitMatrix<N>::zeroMatrix();
		for(int i = 0; i < N; ++i) M.set(i, 0);
		return getLMMZ(M);
	}

	/* 
	 * Function: getRightColumnMatrix
	 * Returns a matrix that is the encrypted version of [ 0 | ... | 0 | 1 ] 
	 * where each of {0,1} here is a column
	 * Dimension: 2*(N * 2^6) by 4*(N * 2^6)
	 */
	const BitMatrix<2*N, 4*N> getRightColumnMatrix() const{
		BitMatrix<N> M = BitMatrix<N>::zeroMatrix();
		for(int i = 0; i < N; ++i) M.set(i, N - 1);
		return getLMMZ(M);
	}

/* Binary unified code */

	/*
	 * Function: getBinaryG1
	 * Returns the first function of the obfuscation chain for binary operations (e.g. XOR, AND)
	 */
	const MultiQuadTuple<4*N, 3*N> getBinaryG1() const{
		const MultiQuadTupleChain<N,2> & f = _pk.getf();

		const BitMatrix<N, 2*N> & M2 = _M.inv().splitV2(1);
		const BitMatrix<N, 4*N> & matTop = BitMatrix<N, 4*N>::augH(M2, BitMatrix<N, 2*N>::zeroMatrix());
		const BitMatrix<N, 4*N> & matMid = BitMatrix<N, 4*N>::augH(BitMatrix<N, 2*N>::zeroMatrix(), M2);
		const BitMatrix<N, 4*N> & matBot = (_Rx * matTop) ^ (_Ry * matMid);

		const MultiQuadTuple<4*N, N> & top = f.get(0) * matTop;
		const MultiQuadTuple<4*N, N> & mid = f.get(0) * matMid;
		const MultiQuadTuple<4*N, N> & bot = f.get(0) * matBot;
		MultiQuadTuple<4*N, 3*N> aug;
		aug.augV(top, mid, bot);
		return aug.template rMult<3*N>(_Cb1);
	}

	/*
	 * Function: getBinaryG2
	 * Returns the second function (g2) of the obfuscation chain for binary operations (e.g. XOR, AND)
	 */
	const MultiQuadTuple<3*N, 3*N>getBinaryG2() const{
		const MultiQuadTupleChain<N,2> & f = _pk.getf();

		const BitMatrix<3*N> & Cb1Inv = _Cb1.inv();
		const BitMatrix<N, 3*N> & matTop = Cb1Inv.splitV3(0);
		const BitMatrix<N, 3*N> & matMid = Cb1Inv.splitV3(1);
		const BitMatrix<N, 3*N> & matBot = Cb1Inv.splitV3(2);

		const MultiQuadTuple<3*N, N> & top = f.get(1) * matTop;
		const MultiQuadTuple<3*N, N> & mid = f.get(1) * matMid;
		const MultiQuadTuple<3*N, N> & bot = f.get(1) * matBot;
		MultiQuadTuple<3*N, 3*N> aug;
		aug.augV(top, mid, bot);
		return aug.template rMult<3*N>(_Cb2);
	}

/* XOR */
	struct H_XOR {
		BitMatrix<2*N> _Xx, _Xy;
		BitMatrix<2*N, 3*N> _Y;
		MultiQuadTuple<4*N, 3*N> _gb1;
		MultiQuadTuple<3*N, 3*N> _gb2;
		void initialize(const BitMatrix<2*N> & Xx, const BitMatrix<2*N> & Xy, const BitMatrix<2*N, 3*N> & Y, const MultiQuadTuple<4*N, 3*N> & gb1, const MultiQuadTuple<3*N, 3*N> & gb2){
			_Xx = Xx;
			_Xy = Xy;
			_Y = Y;
			_gb1 = gb1;
			_gb2 = gb2;
		}
		const BitVector<2*N> operator()(const BitVector<2*N> &x, const BitVector<2*N> &y) const{
			const BitVector<4*N> & concatXY = BitVector<4*N>::template vCat<2*N, 2*N>(x, y);
			const BitVector<3*N> & t = _gb2(_gb1(concatXY));
			return (_Xx * x) ^ (_Xy * y) ^ (_Y * t);
		}
	};

	const H_XOR getXOR() const{
		H_XOR result;
		refreshParam();
		result.initialize(getXORXx(), getXORXy(), getXORY(), getBinaryG1(), getBinaryG2());
		return result;
	}

/* AND */
	struct H_AND {
		BitMatrix<2*N, N> _MB;
		BitMatrix<2*N, 3*N> _MY3;
		MultiQuadTuple<7*N, N> _z;
		BitMatrix<2*N> _Z1, _Z2;
		MultiQuadTuple<4*N, 3*N> _gb1;
		MultiQuadTuple<3*N, 3*N> _gb2;
		void initialize(const BitMatrix<2*N, N> & MB, const BitMatrix<2*N, 3*N> & MY3, const MultiQuadTuple<7*N, N> & z, const BitMatrix<2*N> & Z1, const BitMatrix<2*N> & Z2, const MultiQuadTuple<4*N, 3*N> & gb1, const MultiQuadTuple<3*N, 3*N> & gb2){
			_MB = MB;
			_MY3 = MY3;
			_z = z;
			_Z1 = Z1;
			_Z2 = Z2;
			_gb1 = gb1;
			_gb2 = gb2;
		}
		const BitVector<2*N> operator()(const BitVector<2*N> &x, const BitVector<2*N> &y) const{
			const BitVector<4*N> & concatXY = BitVector<4*N>::template vCat<2*N, 2*N>(x, y);
			const BitVector<3*N> & t = _gb2(_gb1(concatXY));
			const BitVector<7*N> & coordinates = BitVector<7*N>::vCat(x, y, t);
			return (_MB * _z(coordinates)) ^ (_MY3 * t) ^ (_Z1 * x) ^ (_Z2 * y);
		}
	};

	const H_AND getAND() const{
		H_AND result;
		refreshParam();
		const BitMatrix<2*N, N> & MB = _M.splitH2(0); // is this a possible security vulnerability??
		const BitMatrix<2*N, 3*N> & MY3 = _M * BitMatrix<2*N, 3*N>::augV(_Cb2.inv().splitV3(2), BitMatrix<N, 3*N>::zeroMatrix());
		result.initialize(MB, MY3, getANDz(), getANDZ1(), getANDZ2(), getBinaryG1(), getBinaryG2());
		return result;
	}

private:
	const PrivateKey<N> _pk;
	const BitMatrix<N> _R;
	BitMatrix<N> _Rx;
	BitMatrix<N> _Ry;
	const BitMatrix<2*N> _M;
	const BitMatrix<2*N> _Cu1;
	const BitMatrix<2*N> _Cu2;
	const BitMatrix<3*N> _Cb1;
	const BitMatrix<3*N> _Cb2;
	static const unsigned int twoN = N << 1;
	static const unsigned int threeN = 3 * N;

	/*
	 * Function: Refresh and re-randomise Rx, Ry and all associated variables
	 * Returns void
	 */
	void refreshParam() const {
		// re-randomise Rx, Ry
		_Rx.copy(BitMatrix<N>::randomInvertibleMatrix());
		_Ry.copy(BitMatrix<N>::randomInvertibleMatrix());
	}

/* Helper Functions for getXOR */

	/*
	 * Function: getXORXx
	 * Returns matrix Xx used for homomorphic XOR
	 */
	const BitMatrix<2*N> getXORXx() const{
		const BitMatrix<N, 2*N> & XTop = BitMatrix<N, 2*N>::augH(BitMatrix<N>::identityMatrix(), BitMatrix<N>::zeroMatrix());
		const BitMatrix<N, 2*N> & XBot = BitMatrix<N, 2*N>::augH(BitMatrix<N>::zeroMatrix(), _Rx);
		return _M * BitMatrix<2*N>::augV(XTop, XBot) * _M.inv();
	}

	/*
	 * Function: getXORXy
	 * Returns matrix Xy used for homomorphic XOR
	 */
	const BitMatrix<2*N> getXORXy() const{
		const BitMatrix<N, 2*N> XTop = BitMatrix<N, 2*N>::augH(BitMatrix<N>::identityMatrix(), BitMatrix<N>::zeroMatrix());
		const BitMatrix<N, 2*N> XBot = BitMatrix<N, 2*N>::augH(BitMatrix<N>::zeroMatrix(), _Ry);
		return _M * BitMatrix<2*N>::augV(XTop, XBot) * _M.inv();
	}

	/*
	 * Function: getXORY
	 * Returns matrix Y used for homomorphic XOR
	 */
	const BitMatrix<2*N, 3*N> getXORY() const{
		const BitMatrix<N> & idN = BitMatrix<N>::identityMatrix();

		const BitMatrix<N, 3*N> & YTop = BitMatrix<N, 3*N>::augH(idN, idN, idN);
		return _M * BitMatrix<2*N, 3*N>::augV(YTop, BitMatrix<N, 3*N>::zeroMatrix()) * _Cb2.inv();
	}

/* Helper functions for getAND */

	/*
	 * Function: getANDX
	 * Returns matrix X used to compute z for homomorphic AND
	 */
	const BitMatrix<N, 2*N> getANDX() const{
		const BitMatrix<N, 2*N> & inner = BitMatrix<N, 2*N>::augH(BitMatrix<N>::identityMatrix(), BitMatrix<N>::zeroMatrix());
		return inner * _M.inv();
	}

	/*
	 * Function: getANDPk
	 * top chunk of contrib matrix for z for homomorphic AND
	 * level ranges from 0 to 64 * 2N - 1
	 */
	template <unsigned int level>
	const BitMatrix<7*N - level, N> getANDPk(const BitMatrix<N, 2*N> &X, const BitMatrix<N, 3*N> &Y2) const{
		const BitMatrix<twoN - level, N> & top = BitMatrix<twoN - level, N>::zeroMatrix();

		BitMatrix<twoN, N> mid = BitMatrix<twoN, N>::zeroMatrix();
		for (unsigned int j = 0; j < N; ++j) { //col within middle block
			if(X.get(j, level)){
				for (unsigned int i = 0; i < twoN; ++i) { //row within middle block
					mid.set(i, j, X.get(j, i));
				}
			}
		}

		BitMatrix<threeN, N> bot = BitMatrix<threeN, N>::zeroMatrix();
		for (unsigned int j = 0; j < N; ++j) { //col within bottom block
			if(X.get(j, level)){
				for (unsigned int i = 0; i < threeN; ++i) { //row within bottom block
					bot.set(i, j, Y2.get(j, i));
				}
			}
		}
		return BitMatrix<7*N - level, N>::augV(top, mid, bot);
	}

	/*
	 * Function: getANDQk
	 * middle chunk of contrib matrix for z
	 * level ranges from 0 to 64 * 2N - 1
	 */
	template <unsigned int level>
	const BitMatrix<5*N - level, N> getANDQk(const BitMatrix<N, 2*N>&X, const BitMatrix<N, 3*N> &Y1) const{
		const BitMatrix<twoN - level, N> & top = BitMatrix<twoN - level, N>::zeroMatrix();
		BitMatrix<threeN, N> bot = BitMatrix<threeN, N>::zeroMatrix();
		for (unsigned int j = 0; j < N; ++j) { //col within bottom block
			if(X.get(j, level)){
				for (unsigned int i = 0; i < threeN; ++i) { //row within bottom block
					bot.set(i, j, Y1.get(j, i));
				}
			}
		}
		return BitMatrix<5*N - level, N>::augV(top, bot);
	}

	/*
	 * Function: getANDSk
	 * level ranges from 0 to 64 * 3N - 1
	 */
	template <unsigned int level>
	const BitMatrix<3*N - level, N> getANDSk(const BitMatrix<N, 3*N> &Y1, const BitMatrix<N, 3*N> &Y2) const{
		BitMatrix<threeN - level, N> contrib = BitMatrix<threeN - level, N>::zeroMatrix();
		for (unsigned int j = 0; j < N; ++j) { //cols
			bool prod = Y1.get(j, level) && Y2.get(j, level); //first row
			contrib.set(0, j, prod);
			if(Y1.get(j, level)){
				for (unsigned int i = 1; i < threeN - level; ++i) { //rows
					contrib.set(i, j, Y2.get(j, level + i));
				}
			}
			if(Y2.get(j, level)){
				for(unsigned int i = 1; i < threeN - level; ++i){
					contrib.set(i, j, contrib.get(i, j) ^ Y1.get(j, level + i));
				}
			}
		}
		return contrib;
	}

	/*
	 * Function: getANDP (entire chunk)
	 * top chunk of contrib matrix for z for homomorphic AND
	 */
	const BitMatrix<N*(12*N + 1), N> getANDP(const BitMatrix<N, 2*N> &X, const BitMatrix<N, 3*N> &Y2) const{
		BitMatrix<N*(12*N + 1), N> contrib = BitMatrix<N*(12*N + 1), N>::zeroMatrix();
		unsigned int count = 0;
		//unsigned int toAdd = 7 * N;
		for(unsigned int level = 0; level < twoN; ++level){
			/****************************************************************/
			//BitMatrix<twoN - level, N> top = BitMatrix<twoN - level, N>::zeroMatrix();

			BitMatrix<twoN, N> mid = BitMatrix<twoN, N>::zeroMatrix();
			for (unsigned int j = 0; j < N; ++j) { //col within middle block
				if(X.get(j, level)){
					for (unsigned int i = 0; i < twoN; ++i) { //row within middle block
						mid.set(i, j, X.get(j, i));
					}
				}
			}

			contrib.setSubMatrix(count + twoN - level, mid);

			BitMatrix<threeN, N> bot = BitMatrix<threeN, N>::zeroMatrix();
			for (unsigned int j = 0; j < N; ++j) { //col within bottom block
				if(X.get(j, level)){
					for (unsigned int i = 0; i < threeN; ++i) { //row within bottom block
						bot.set(i, j, Y2.get(j, i));
					}
				}
			}

			contrib.setSubMatrix(count + twoN - level + twoN, bot);

			count += (7*N - level);
			/****************************************************************/
			//contrib.setSubMatrix(count, getANDPk<level>(X, Y2));
			//count += toAdd;
			//--toAdd;
		}
		return contrib;
	}

	/*
	 * Function: getANDQ (entire chunk)
	 * middle chunk of contrib matrix for z for homomorphic AND
	 */
	const BitMatrix<N*(8*N + 1), N> getANDQ(const BitMatrix<N, 2*N> &X, const BitMatrix<N, 3*N> &Y1) const{
		BitMatrix<N*(8*N + 1), N> contrib = BitMatrix<N*(8*N + 1), N>::zeroMatrix();
		unsigned int count = 0;
		//unsigned int toAdd = 5 * N;
		for(unsigned int level = 0; level < twoN; ++level){
			/****************************************************************/
			BitMatrix<threeN, N> bot = BitMatrix<threeN, N>::zeroMatrix();
			for (unsigned int j = 0; j < N; ++j) { //col within bottom block
				if(X.get(j, level)){
					for (unsigned int i = 0; i < threeN; ++i) { //row within bottom block
						bot.set(i, j, Y1.get(j, i));
					}
				}
			}
			contrib.setSubMatrix(count + twoN - level, bot);
			count += (5*N - level);
			/****************************************************************/
			//contrib.setSubMatrix(count, getANDQk<level>(X, Y1));
			//count += toAdd;
			//--toAdd;
		}
		return contrib;
	}

	/*
	 * Function: getANDS (entire chunk)
	 * bottom chunk of contrib matrix for z
	 */
	const BitMatrix<((3*N*(3*N + 1)) >> 1), N> getANDS(const BitMatrix<N, 3*N> &Y1, const BitMatrix<N, 3*N> &Y2) const{
		BitMatrix<((3*N*(3*N + 1)) >> 1), N> contrib = BitMatrix<((3*N*(3*N + 1)) >> 1), N>::zeroMatrix();
		unsigned int count = 0;
		//unsigned int toAdd = 3 * N;
		for(unsigned int level = 0; level < threeN; ++level){
			/****************************************************************/
			for (unsigned int j = 0; j < N; ++j) { //cols
				bool prod = Y1.get(j, level) && Y2.get(j, level); //first row
				contrib.set(count, j, prod);
				if(Y1.get(j, level)){
					for (unsigned int i = 1; i < threeN - level; ++i) { //rows
						contrib.set(i+count, j, Y2.get(j, level + i));
					}
				}
				if(Y2.get(j, level)){
					for(unsigned int i = 1; i < threeN - level; ++i){
						contrib.set(i+count, j, contrib.get(i+count, j) ^ Y1.get(j, level + i));
					}
				}
			}
			count += (3*N - level);
			/****************************************************************/
			//contrib.setSubMatrix(count, getANDSk<level>(Y1, Y2));
			//count += toAdd;
			//--toAdd;
		}
		return contrib;
	}

	/*
	 * Function: getANDz
	 * Returns function tuple z used for homomorphic AND
	 */
	const MultiQuadTuple<7*N, N> getANDz() const{
		const BitMatrix<N, 2*N> & X = getANDX();
		const BitMatrix<3*N> & Cb2i = _Cb2.inv();
		const BitMatrix<N, 3*N> & Y1 = Cb2i.splitV3(0);
		const BitMatrix<N, 3*N> & Y2 = Cb2i.splitV3(1);
		const BitMatrix<((7*N * (7*N + 1)) >> 1), N> & contrib = BitMatrix<((7*N * (7*N + 1)) >> 1), N>::augV(getANDP(X, Y2), getANDQ(X, Y1), getANDS(Y1, Y2));
		MultiQuadTuple<7*N, N> z;
		z.setContributions(contrib, BitVector<N>::zeroVector());
		return z;
	}

	/*
	 * Function: getANDZ1
	 * Returns matrix Z1 used for homomorphic AND
	 */
	const BitMatrix<2*N> getANDZ1() const{
		const BitMatrix<N, 2*N> & top = BitMatrix<N, 2*N>::zeroMatrix();
		const BitMatrix<N, 2*N> & bottom = _Rx * _M.inv().splitV2(1);
		return _M * BitMatrix<2*N>::augV(top, bottom);
	}

	/*
	 * Function: getANDZ2
	 * Returns matrix Z2 used for homomorphic AND
	 * Dimension of Z2: 2*(N * 2^6) by 2*(N * 2^6)
	 */
	const BitMatrix<2*N> getANDZ2() const{
		const BitMatrix<N, 2*N> & top = BitMatrix<N, 2*N>::zeroMatrix();
		const BitMatrix<N, 2*N> & bottom = _Ry * _M.inv().splitV2(1);
		return _M * BitMatrix<2*N>::augV(top, bottom);
	}

};

#endif