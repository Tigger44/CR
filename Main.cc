#include "Main.h" 
#include "Local.h"

using std::cout;
using std::endl;

int main(){

    std::cout << "***COSMIC RAYS***\n\n";

    double finalTime, h, h0, err;
    bool adapFlag,stopFlag;

    double q, m, initialB;
    int mode;

    std::vector<double> pos(3), v(3), B(3);

    PhysInit(finalTime,h,err,q,m,pos,v,B,initialB,mode,adapFlag,stopFlag);

    std::vector<double> pos0(3), v0(3), B0(3);
    for(int i = 0; i < 3; i++){     // Saves the starting values for future use
        pos0[i] = pos[i];
        v0[i] = v[i];
        B0[i] = B[i];
    }
    h0 = h;

    std::vector<double> maxValues(3), minValues(3);
    for(int i = 0; i < 3; i++){         //Match the values of max and min with 
        maxValues[i] = pos[i];          //the starting values
        minValues[i] = pos[i];
    }

    m = RelativisticMass(m,v);
    std::cout << "mass = " <<  m << std::endl;

//#############################################################################
//                  Store intitial values of the metrics
//#############################################################################

    MagneticField(pos,B,B0,initialB,mode);

    std::vector<double> timeStamp(1);
    timeStamp[0] = 0;

    std::vector<std::vector<double>> posOut(1, std::vector<double>(3));
    for(int i = 0; i < 3; i++){
        posOut[0][i] = pos[i];
    }

    std::vector<double> radius(1);
    radius[0] = Gyroradius(q,m,v,B);

    std::vector<std::vector<double>> magfield(1, std::vector<double>(3));
    for(int i = 0; i < 3; i++){
        magfield[0][i] = B[i];
    }

    std::vector<double> frequency(1);
    frequency[0] = Frequency(q,m,v,B);

//#############################################################################
//                               RK4
//#############################################################################

    int adapCounter = 0;       //variables for the adaptative method
    double adapTime;
    std::vector<double> adapPosComp(3), adapVComp(3);
    std::vector<double> adapPos(3), adapV(3);
    std::vector<double> adapMaxValues(3), adapMinValues(3);

    for(double time = 0; time <= finalTime || adapCounter != 0;){         

        if(adapFlag){
            if(adapCounter == 0){
                adapPos = pos;
                adapV = v;

                adapMaxValues = maxValues;
                adapMinValues = minValues;
                adapTime = time;
                adapCounter++;
            }else if(adapCounter == 1){
                adapCounter++;
            }else if(adapCounter == 2){
                adapCounter = 0;

                adapPosComp = adapPos;
                adapVComp = adapV;
                Rk(adapPosComp,adapVComp,B,B0,initialB,mode,q,m,2*h);

                switch(RkCompare(pos,adapPosComp,h,h0,err)){
                    case 1:
                        time = adapTime; 
                        pos = adapPos;
                        v = adapV;
                        maxValues = adapMaxValues;
                        minValues = adapMinValues;

                        timeStamp.resize(timeStamp.size()-2);
                        posOut.resize(posOut.size()-2);
                        radius.resize(radius.size()-2);
                        frequency.resize(frequency.size()-2);
                        magfield.resize(magfield.size()-2);
                        continue;

                    default:
                        break;
                }
            }
        }

        Rk(pos,v,B,B0,initialB,mode,q,m,h);

        std::vector<double> temp(3);    //temporary vec to be used to push
                                        //into vector of vectors
        for(int i = 0; i < 3; i++){
            if(pos[i] != pos[i]){       //comparing two nan value is false
                std::cout << "Nan value" << std::endl;
                return 1;
            }
            temp[i] = pos[i];
        }

        posOut.push_back(temp);

        radius.push_back(Gyroradius(q,m,v,B));
        frequency.push_back(Frequency(q,m,v,B));

        for(int i = 0; i < 3; i++){
            temp[i] = B[i];
        }
        magfield.push_back(temp);

        for(int i = 0; i < 3; i++){
            maxValues[i] = Max(maxValues[i],pos[i]);
            minValues[i] = Min(minValues[i],pos[i]);
        }

        if(stopFlag){
            if(SimStop(B)){
                std::cout << "SimStop Trigered" << std::endl;
                break;
            }
        }

        time += h;
        timeStamp.push_back(time);
        cout << "time = " << time << endl;
    }

//#############################################################################
//                      Print values to .dat files
//#############################################################################

    printPos(timeStamp, maxValues, minValues, posOut);
    printRad(timeStamp, radius);
    printFreq(timeStamp, frequency);
    printMag(timeStamp, magfield);

    return 0;
}
