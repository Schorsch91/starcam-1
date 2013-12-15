#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

#define MAGNITUDE_THRESHOLD 6

using namespace std;

struct in {
        double id;
        double ra;
        double dec;
        double mag;
};

struct tab1 {
        double id;
        double x;
        double y;
        double z;
        double mag;
};

struct tab2 {
        double id1;
        double id2;
        double id3;
        double alpha1;
        double alpha2;
        double beta;
};

struct coordinates3d {
        double x;
        double y;
        double z;
};

struct angle_triple {
        double alpha1;
        double alpha2;
        double beta;
};

struct coordinates
{
        int x = -1;
        int y = -1;
};

struct doubleCoordinates
{
        double x = -1.0;
        double y = -1.0;
};

struct cluster
{
        coordinates maxPoint;
        doubleCoordinates gravCenter;
        int size = 1; 
        int maxBrightness;
        double relBrightness;
        vector<coordinates> points;
};

struct quaternion {
        double q0;
        double q1;
        double q2;
        double q3;
};

struct vector3d {
        double x;
        double y;
        double z;
};

class Database
{
public:
        Database(string inputfile, string outputfile_table1, string outputfile_table2, double foc_len, double pix_size);
        
        //sucht alpha1, alpha2, beta in tabelle 2
        bool find_triple (angle_triple key, tab2 *result);
        vector3d coordsOfStarWithID (double id);
        
        
private:
        vector<in> input_data;
        vector<tab1> dataTable1;
        vector<tab2> dataTable2;

        double focalLength;
        double pixelSize;

        void importInputfile(string filename);
        void generateTable1(string filename);
        void generateTable2(string filename);

        double calcStarCamDistance(coordinates3d c);
        double calcDistanceBetweenStars(coordinates3d a, coordinates3d b);
        double calcAlphaAngle(double distStarNeighbour, double distCamStar, double distCamNeighbourStar);
        double calcBetaAngle(coordinates3d c1, coordinates3d c2, double dist1, double dist2);

};
