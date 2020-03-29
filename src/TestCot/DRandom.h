/********************************************************************
    created:    2011/09/01 22:33
    file:       DRandom.h
    author:     Foreverflying
    purpose:    
********************************************************************/

#ifndef __DRandom_h__
#define __DRandom_h__

class DRandom
{
public:
    explicit DRandom(unsigned long s) : seed(s)
    {
        randomize();
    }
    void reset(unsigned long s)
    {
        seed = s;
        randomize();
    }
    unsigned long rand(unsigned long low = 0, unsigned long high = MAX_NUM)
    {
        // returns a random integer in the range [low,high)
        // if low > high, the result would be undefined
        randomize();
        return high > low ? seed % (high - low) + low : low;
    }
    double real()
    {
        // returns a random real number in the range [0.0, 1.0)
        randomize();
        return (double)(seed) / MAX_NUM;
    }
private:
    unsigned long seed;
    static const unsigned long MAX_NUM = (unsigned long)-1;
    void randomize() { seed = (1103515245UL * seed + 123456789UL) % MAX_NUM; }
};

#endif // __DRandom_h__
