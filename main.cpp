#include "mbed.h"

AnalogIn photocell[] = {p20, p19, p18, p17, p16, p15};
DigitalOut myled[] ={LED4,LED3,LED2,LED1,p5};
float minLUx[] = {10.0, 10.0,10.0,10.0,10.0};
float maxLUx[] = {-10.0,-10.0,-10.0,-10.0,-10.0}; 
float critical[] = {0.3,0.3,0.45,0.3825,0.3}; 
DigitalOut light (p6); 
DigitalOut leftTurnSignal (p9); 
DigitalOut RightTurnSignal (p10); 
DigitalIn button(p8);
int mode=0;//0:train, 1:run
int runMode=0;
LocalFileSystem local("local");  

PwmOut speedLeft (p21);
PwmOut speedRight (p22);
DigitalOut DirectionLeft[] = {p7,p11};
DigitalOut DirectionRight[] = {p12,p13};

int numOfIndex = 20;
float K =0.05;
float A =1;
float B =1.05;
float endTime=1.5;
float prevDirection=0.0;
float inDirection=0.0;
float direction=0.0;
float outDirection=0.0;
int midEmptyCt = 0;
int midNonEmptyCt = 0;
int scoreCt = 0;

float leftTime =0.0;
float rightTime =0.0;

float toPercent(int sensorIndex, float Lux)
    {
        return (Lux-minLUx[sensorIndex])/(maxLUx[sensorIndex]-minLUx[sensorIndex]);
    }
    
void LeftGoFront(float speed)
    {
        DirectionLeft[0] = 1;
        DirectionLeft[1] = 0;
        speedLeft=speed;
    }

void LeftGoBack(float speed)
    {
        DirectionLeft[0] = 0;
        DirectionLeft[1] = 1;
        speedLeft=speed;
    }
    
void RightGoFront(float speed)
    {
        DirectionRight[0] = 0;
        DirectionRight[1] = 1;
        speedRight=speed;
    }

void RightGoBack(float speed)
    {
        DirectionRight[0] = 1;
        DirectionRight[1] = 0;
        speedRight=speed;
    }

void Stop()
{
    speedRight=0.0;
    speedLeft=0.0;
}

void goStraight()  
{
    RightTurnSignal=0;
    leftTurnSignal=0;
    /*
    float currSpeed=K/(-exp(-A*t))+B;
    if (currSpeed<0.0)
    {
        currSpeed=0.0;
    }
    */
    LeftGoFront(0.25*0.525);
    RightGoFront(0.25*0.525);
    //wait(0.01/(numOfIndex*1.0));
    //Stop();
}

void rotateRight()
{
    float currSpeed=K/(-exp(-A*rightTime))+B;
    rightTime+=0.01/(5.0);
    rightTime= rightTime>=endTime ? endTime : rightTime;
    RightTurnSignal=1;
    leftTurnSignal=0;
    LeftGoFront(0.25*1.25*currSpeed);//0.4
    RightGoBack(0.4*1.25*currSpeed);//0.5
    wait(0.01/(5.0));
    Stop();
}

void rotateLeft()
{
    float currSpeed=K/(-exp(-A*leftTime))+B;
    leftTime+=0.01/(5.0);
    leftTime= leftTime>=endTime ? endTime : leftTime;
    RightTurnSignal=0;
    leftTurnSignal=1;
    RightGoFront(0.25*1.25*currSpeed);//0.4
    LeftGoBack(0.4*1.25*currSpeed);//0.5
    //wait(0.01);
    wait(0.01/(5.0));
    Stop();
}

void turnLeft()
{
    RightTurnSignal=0;
    leftTurnSignal=0;
    RightGoFront(0.25*3*0.38);//0.4
    speedLeft=0;
    //LeftGoFront(0.25*1.2*0.4);//0.5
    //wait(0.01);
    //wait(0.01/(numOfIndex*1.0));
    //Stop();
}

void turnRight()
{
    RightTurnSignal=0;
    leftTurnSignal=0;
    //RightGoFront(0.25*1.2*0.4);//0.4
    speedRight=0;
    LeftGoFront(0.25*3*0.38);//0.5
    //wait(0.01);
    //wait(0.01/(numOfIndex*1.0));
    //Stop();
}

int main()
{
    FILE *fp;
    mode = 1;
    button.mode(PullUp);
    int prevIndex[5];
    int ct=0;
    float minPercent;
    float currPercent;
    int minIndex;
    int maxPosb;
    int maxPosbIndex;
    float currLUX;
    light =1;
    int time=0;
    int UIpos=-1;
    int lightsCt=0;
    //read calibration from local
    fp = fopen("/local/CALIBRAT.txt", "read_write_append_mode");
    char buf [80]; 
    for (int i=0;i<5;i++)
    {
        fscanf(fp, "%s", &buf);
        minLUx[i]=atof(buf);
    }
    for (int i=0;i<5;i++)
    {
        fscanf(fp, "%s", &buf);
        maxLUx[i]=atof(buf);
    }
    fclose (fp);
    
    while(1) 
    {
        if (mode == 0)
        {
            if (time%5==0)
            {
                UIpos = UIpos==3? 0:UIpos+1;
                myled[UIpos]=1;
                myled[UIpos-1==-1? 3:UIpos-1]=0;
            }
            for (int i=0;i<5;i++)
            {
                currLUX =photocell[i];
                if (minLUx[i]>currLUX)
                {
                    minLUx[i]=currLUX;
                }
                if (maxLUx[i]<currLUX)
                {
                    maxLUx[i]=currLUX;
                }
            }
            if (button == 0)
            {
                time=0;
                mode = 1;
                myled[UIpos]=0;
                wait(1);
                //save calibration to local
                fp = fopen("/local/CALIBRAT.txt", "w");
                char buf [80]; 
                for (int i=0;i<5;i++)
                {
                    sprintf(buf, "%.6f", minLUx[i]);
                    fprintf(fp, buf);
                    fprintf(fp, "\n");
                }
                for (int i=0;i<5;i++)
                {
                    sprintf(buf, "%.6f", maxLUx[i]);
                    fprintf(fp, buf);
                    fprintf(fp, "\n");
                }
                fclose(fp);
            }
            else
            {
                time++;
                wait (0.1);
            }
        }
        else
        {
            //run
            if (button == 0)
            {
                Stop();
                for (int i=0;i<5;i++)
                {
                    minLUx[i]=10.0;
                    maxLUx[i]=-10.0;
                }
                mode = 0;
                wait(1);
            }
            else
            {
                if (ct==0)
                {
                    for (int i=0;i<5;i++)
                    {
                        prevIndex[i]=0;
                    }
                }
                for (int i=0;i<5;i++)
                {
                    currLUX =photocell[i];
                    currPercent=toPercent(i,currLUX);
                    prevIndex[i]+=currPercent<=critical[i]? 1:0;
                }        
                if (ct == numOfIndex)
                {
                    //printf("0: %.6f, 1: %.6f, 2: %.6f, 3: %.6f, 4: %.6f\n", toPercent(0, photocell[0]), toPercent(1, photocell[1]),toPercent(2, photocell[2]),toPercent(3, photocell[3]),toPercent(4, photocell[4]));
                    ct=0;
                    lightsCt=0;
                    for (int i=0;i<5;i++)
                    {
                        myled[i] = (prevIndex[i]*1.0) >= (numOfIndex*1.0/2.0) ? 1:0;
                        
                    }        
                    //decide direction
                    if (runMode==0)
                    {
                        scoreCt++;
                        if (myled[2]==1)
                        {
                            midEmptyCt=0;
                            midNonEmptyCt++;
                            if (midNonEmptyCt>50)
                            {
                                leftTime=0;
                                rightTime=0;
                            }
                            if(myled[1]==1 && myled[3]==0)
                            {
                                //rotateLeft();
                                turnLeft();
                            }
                            else if (myled[1]==0 && myled[3]==1)
                            {
                                //rotateLeft();
                                turnRight();
                            }
                            else
                            {
                                goStraight();
                            }
                            
                            if (myled[1]==1)
                            {
                                inDirection=1;
                            }
                            if (myled[3]==1)
                            {
                                inDirection=-1;
                            }
                            
                            if (myled[0]==1)
                            {
                                outDirection=1;
                            }
                            if (myled[4]==1)
                            {
                                outDirection=-1;
                            }
                        }
                        else
                        {
                            midEmptyCt++;
                            if (midEmptyCt<20)
                            {
                                goStraight();
                                if (myled[1]==1)
                                {
                                    inDirection=1;
                                }
                                if (myled[3]==1)
                                {
                                    inDirection=-1;
                                }
                                if (myled[0]==1)
                                {
                                    outDirection=1;
                                }
                                if (myled[4]==1)
                                {
                                    outDirection=-1;
                                }
                            }
                            else if (outDirection!=0)
                            {
                                midNonEmptyCt=0;
                                direction=outDirection;
                                runMode=1;
                            }
                            else if (inDirection!=0)
                            {
                                midNonEmptyCt=0;
                                direction=inDirection;
                                runMode=1;
                            }
                            else
                            {
                                midNonEmptyCt=0;
                                direction=prevDirection;
                                runMode=1;
                            }
                        }
                    }
                    if(runMode==1)
                    {
                        if (direction>0)
                        {
                            rotateLeft();
                        }
                        else if (direction<0)
                        {
                            rotateRight();
                        }
                        else
                        {
                            if (prevDirection>0)
                            {
                                rightTime=0;
                                rotateLeft();
                            }
                            else
                            {
                                leftTime=0;
                                rotateRight();
                            }
                        }
                        if (myled[2]==1 && myled[3]+myled[1]==1)
                        {
                            scoreCt=0;
                            prevDirection=direction;
                            direction=0;
                            runMode=0;
                        }
                    }
                    //
                }
                else
                {
                    ct++;
                    //wait(0.01/(numOfIndex*1.0)/(numOfIndex*1.0));
                }         
            }
        }
    }
    
    //
}


