#include "LogicRoutineManager.h"

long LogicRoutineManager::lastTimestamp = -1L;

LogicRoutineManager::LogicRoutineManager():
			sequence(0),
		    workerId(0),
		    regionId(0)
{

}

long LogicRoutineManager::GetNextRid(bool isPadding, long busId)
{
	//生成唯一rid
    long timestamp = timeGen();
    long paddingnum = regionId;

    //41 bits: Timestamp (毫秒) | 3 bits: 区域（机房） | 10 bits: 机器编号 | 10 bits: 序列号 |
    if (isPadding)
    {
         paddingnum = busId;
    }

    if (timestamp < lastTimestamp)
    {
        try
        {
            throw runtime_error("Clock moved backwards.  Refusing to generate id for milliseconds");
        }catch (exception & e)
        {
            e.what();
        }
    }

    //如果上次生成时间和当前时间相同,在同一毫秒内
    if (lastTimestamp == timestamp)
    {
        //sequence自增，因为sequence只有10bit，所以和sequenceMask相与一下，去掉高位
        sequence = (sequence + 1) & sequenceMask;
        //判断是否溢出,也就是每毫秒内超过1024，当为1024时，与sequenceMask相与，sequence就等于0
        if (sequence == 0)
        {
            //自旋等待到下一毫秒
            timestamp = tailNextMillis(lastTimestamp);
        }
    } else
    {
        // 如果和上次生成时间不同,重置sequence，就是下一毫秒开始，sequence计数重新从0开始累加,
        // 为了保证尾数随机性更大一些,最后一位设置一个随机数
        sequence = rand()%10;
    }

    lastTimestamp = timestamp;

 //   cout<<timestamp<<","<<twepoch<<",seq="<<sequence<<endl;
    return ((timestamp - twepoch) << timestampLeftShift) | (paddingnum << regionIdShift) | (workerId << workerIdShift) | sequence;
}

long LogicRoutineManager::tailNextMillis(const long lastTimestamp)
{
    long timestamp = timeGen();
    while (timestamp <= lastTimestamp) {
        timestamp = timeGen();
    }
    return timestamp;
}

long LogicRoutineManager::timeGen()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


