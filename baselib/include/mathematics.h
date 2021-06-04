#ifndef MATHEMATICS_H
#define MATHEMATICS_H

class CMath
{
public:

	/*!
	 * \brief 产生一个随机数(内部自动设置种子)
	*/
	static int Random();

	/*!
	 * \brief 产生一个随机数[0, max)(内部自动设置种子)
	*/
	static int RandomInt(int nMax);

    /*!
     * \brief 四舍五入到个位前nBit位
     * \param[in] nData:
     * \param[in] nBits: [ >=0--位数[0-个位 1-十位 ...]
     * \return int [ 四舍五入后的数据 ]
     * 注:本函数采用整型数据运算,保证了整型数据的精确性
    */
    static int RoundInt(int nData, int nBits);

    /*!
     * \brief 四舍五入到个位前nBit位
     * \param[in] dData:
     * \param[in] nBits: [ >=0--位数[0-个位 1-十位 ...]
     * \return int [ 四舍五入后的数据 ]
     * 注: 请调用者保证nBits不小于0
     * 注: 本函数涉及浮点运算,不保证数据类型带来的误差可能造成的错误,如0.5由于精度原因可能会进一也可能舍去
    */
    static int RoundInt(double dData, int nBits);

    /*!
     * \brief 四舍五入到个位前nBit位,本函数涉及浮点运算,不保证数据类型带来的误差可能造成的错误
     * \param[in] dData:
     * \param[in] nBits: 位数[0-个位 1-十位 -1-十分位 ...]
     * \return double [ 四舍五入后的数据 ]
     * 注: 本函数涉及浮点运算,不保证数据类型带来的误差可能造成的错误,如0.5由于精度原因可能会进一也可能舍去
    */
    static double RoundDouble(double dData, int nBits);
};

#endif

