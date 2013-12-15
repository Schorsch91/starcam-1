#include "database.h"

using namespace std;

vector<unsigned char> header;
coordinates imageDimension;
coordinates imageCenter;

bool** outputMatrix;
	

void printCoord(coordinates c, string s = ""){
	printf("%s(%d|%d)\n",s.c_str(), c.x, c.y);
}

void printCoord(doubleCoordinates c, string s = ""){
	printf("%s(%f|%f)\n",s.c_str(), c.x, c.y);
}

void printCluster(cluster c, string s = ""){
	printf("Brightness %d (rel %f) at ", c.maxBrightness, c.relBrightness);
	printCoord(c.maxPoint);
	printf("Size %d around gravity center ", c.size);
	printCoord(c.gravCenter);
	printf("Contains %lu pixel\n\n", c.points.size());
}

#pragma mark - check functions

bool isValidPixel(coordinates pixel, bool checkForValidPrinting = false){
	if(checkForValidPrinting){
		return !(pixel.x < 1 || pixel.x > (imageDimension.x - 1) || 
			pixel.y < 1 || pixel.y > (imageDimension.y - 1));
	} else{
		return !(pixel.x < 0 || pixel.x > imageDimension.x || 
			pixel.y < 0 || pixel.y > imageDimension.y);
	}
}

bool isValidPixel(doubleCoordinates pixel, bool checkForValidPrinting = false){
	if(checkForValidPrinting){
		return !(pixel.x < 1 || pixel.x > (imageDimension.x - 1) || 
			pixel.y < 1 || pixel.y > (imageDimension.y - 1));
	} else{
		return !(pixel.x < 0 || pixel.x > imageDimension.x || 
			pixel.y < 0 || pixel.y > imageDimension.y);
	}
}

bool coordAreEqual(coordinates a, coordinates b){
	return (a.x == b.x) && (a.y == b.y);
}

bool coordAreEqual(doubleCoordinates a, coordinates b){
	return ((int)a.x == b.x) && ((int)a.y == b.y);
}

#pragma mark - matrix creations

int** createIntegerMatrix(coordinates dimension){
	int** m = new int*[dimension.x];
	for(int i = 0; i < dimension.x; i++){
		m[i] = new int[dimension.y];
	}
	return m;
}

bool** createBinaryMatrix(coordinates dimension){
	bool** m = new bool*[dimension.x];
	for(int i = 0; i < dimension.x; i++){
		m[i] = new bool[dimension.y];
	}
	return m;
}

#pragma mark - file handling

vector<unsigned char> getImageHeader(string filename){
	vector<unsigned char> header;
	ifstream in;
    in.open(filename, fstream::binary);
    
    int start = 0;
    for(int a=0; a<10; a++)
	{
    	in.get();
	}
    start = in.get();
    
    in.seekg(0, ios_base::beg);
    for (int i=0; i < start; i++){
        header.push_back(in.get());
    }
    
    in.close();
    return header;
}

coordinates getPicDimensions(vector<unsigned char> header){
	int width;
	int height;
	int c;
	
	c = header[18];
	width = c;
	c = header[19];
	width = c << 8 | width;
	c = header[20];
	width = c << 16 | width;
	c = header[21];
	width = c << 24 | width;
	
	c = header[22];
	height = c;
	c = header[23];
	height = c << 8 | height;
	c = header[24];
	height = c << 16 | height;
	c = header[25];
	height = c << 24 | height;
	
	coordinates coord;
	coord.x = width;
	coord.y = height;
	printCoord(coord, "Imagedimension: ");
	
	imageCenter.x = width / 2;
	imageCenter.y = height / 2;

	return coord;
}

string appendToFilename(string filename, string addition){
	int pos = filename.find_first_of(".");
	return filename.insert(pos, addition);
}

void writeGreyscaleMatrix(int** matrix, vector<unsigned char> header, string filename){

	ofstream out;
	string outputFilename = appendToFilename(filename, "GS");
	
	out.open(outputFilename);
	
	for(int i = 0; i < header.size(); i++){
		out.put(header[i]);
	}

	for(int y = 0; y < imageDimension.y; y++){
		for(int x = 0; x < imageDimension.x; x++){
			out.put(matrix[x][y]);
			out.put(matrix[x][y]);
			out.put(matrix[x][y]);	
		}
	}
	
	out.close();
	printf("greyscale %s written\n", outputFilename.c_str());
}

void writeBWMatrix(bool** matrix, vector<unsigned char> header, string filename){

	ofstream out;
	string outputFilename = appendToFilename(filename, "BW");
	
	out.open(outputFilename);
	
	for(int i = 0; i < header.size(); i++){
		out.put(header[i]);
	}

	for(int y = 0; y < imageDimension.y; y++){
		for(int x = 0; x < imageDimension.x; x++){
			if(matrix[x][y]){
				out.put(255);
				out.put(255);
				out.put(255);	
			}else{
				out.put(0);
				out.put(0);
				out.put(0);
			}
		}
	}
	
	out.close();
	printf("bw %s written\n", outputFilename.c_str());
}

int** createBrightnessMatrix(string filename, int headerSize) {
        
        int** intM = createIntegerMatrix(imageDimension);
        int val = 0, cnt = 0;
        printf("read %s..\n", filename.c_str());
        ifstream in;
        in.open(filename, fstream::binary);
       	in.seekg(0,ios_base::beg);
	
        for (int i=0; i < headerSize; i++){
			in.get();
		}
		for (int y = 0; y < imageDimension.y; y++){
    		for (int x = 0; x < imageDimension.x; x++) {
                val = in.get();
                val += in.get();
                val += in.get();
                intM[x][y] = val/3;

                if(intM[x][y] > 0){
                	cnt++;
                }
            }
        }
        
        in.close();
        printf("%d (%.2f%%) non-black pixels\n", 
        	cnt, (double)cnt/(imageDimension.y * imageDimension.x));
        return intM;
}

int findMaxBrightness(int** brightnessMatrix){
	int curMaxBrightness = 0;
	coordinates point;

	for (int y = 0; y < imageDimension.y; y++){
		for(int x = 0; x < imageDimension.x; x++){
			if(brightnessMatrix[x][y] > curMaxBrightness){
				curMaxBrightness = brightnessMatrix[x][y];
				point.y = y;
				point.x = x;
			}
		}
	}
	printf("max. brightness %d at ", curMaxBrightness);
	printCoord(point);
	return curMaxBrightness;
}

void denoiseMatrix(int** brightnessMatrix, int brightnessThreshold = 20){
	int cnt = 0;

	for (int y = 0; y < imageDimension.y; y++){
		for(int x = 0; x < imageDimension.x; x++){
			if(brightnessMatrix[x][y] != 0 && brightnessMatrix[x][y] < brightnessThreshold){
				brightnessMatrix[x][y] = 0;
				cnt++;
			}
		}
	}
	printf("%d pixels below brightnessThreshold %d \n", cnt, brightnessThreshold);
}

bool pointHasNeighbour(coordinates point, int** matrix){
	
	bool returnVal = false;
	if(point.x > 0)
		returnVal = returnVal || (matrix[point.x - 1][point.y] > 0);
	if(point.y > 0)
		returnVal = returnVal || (matrix[point.x - 1][point.y - 1] > 0);
	if(point.x < imageDimension.x)
		returnVal = returnVal || (matrix[point.x - 1][point.y - 1] > 0);
	if(point.y < imageDimension.y)
		returnVal = returnVal || (matrix[point.x - 1][point.y - 1] > 0);
	return returnVal;
}

bool pointIsEqualToOneOther(coordinates point, coordinates left, coordinates upperLeft, coordinates up, coordinates upperRight){
	return coordAreEqual(point, left) || coordAreEqual(point, up) ||
				coordAreEqual(point, upperLeft) || coordAreEqual(point, upperRight);
}

bool pointIsEqualToOneOther(doubleCoordinates point, coordinates left, coordinates upperLeft, coordinates up, coordinates upperRight){
	return coordAreEqual(point, left) || coordAreEqual(point, up) ||
				coordAreEqual(point, upperLeft) || coordAreEqual(point, upperRight);
}

bool hasValidGravCenter(cluster c){
	return (c.gravCenter.x != -1.0 && isValidPixel(c.gravCenter));
}

int getClusterIndexOfNeighbour(coordinates point, vector<cluster> clusters){
	bool left = point.x > 0;
	bool up = point.y > 0;
	bool upperLeft = left && up;
	bool upperRight = up && (point.y < imageDimension.y);

	coordinates leftCoord, upCoord, upperLeftCoord, upperRightCoord;
	
	if(left){
		leftCoord.x = point.x - 1;
		leftCoord.y = point.y;
	}
	if(up){
		upCoord.x = point.x;
		upCoord.y = point.y - 1;
	}
	if(upperLeft){
		upperLeftCoord.x = point.x - 1;
		upperLeftCoord.y = point.y - 1;
	}
	if(upperRight){
		upperRightCoord.x = point.x + 1;
		upperRightCoord.y = point.y - 1;
	}

	for(int i=0; i < clusters.size(); i++){
		for(int j = 0; j < clusters[i].points.size(); j++){
			if(pointIsEqualToOneOther(clusters[i].points[j], leftCoord, upperLeftCoord, upCoord, upperRightCoord))
				return i;
		}
	}

	return -1;
}

int getClusterIndexOfPoint(coordinates point, vector<cluster> clusters){
	for(int i = 0; i < clusters.size(); i++){
		if(coordAreEqual(clusters[i].gravCenter, point))
			return i;
	}

}

int calcClusterSize(double relBrightness){
	if(relBrightness >= 0.75)
		return 8;
	else if(relBrightness >= 0.5)
		return 6;
	else if(relBrightness >= 0.25)
		return 4;
	else
		return 2;
}

void surroundPointWithBool(bool value, coordinates point, bool** matrix, int radius = 1){
	for (int i = -radius; i <= radius; i++){
		for (int j = -radius; j <= radius; j++){
			matrix[point.x + i][point.y + j] = value;
		}
	}	
}

void writePointWithRadius(coordinates point, int radius, bool** matrix){
	switch(radius){
		case 1:
				matrix[point.x][point.y] = true;
				break;
		case 2:
		case 4:
		case 6:
		case 8:
				//matrix[point.x][point.y] = true;
				surroundPointWithBool(true, point, matrix, radius - 1);
				break;
		default:
				printf("Radius: %d Coord: ", radius);
				printCoord(point);
				printf("Error writing point to matrix!\n");
	}
}

coordinates findMaxBrightnessPoint(cluster c, int** brightnessMatrix){
	int maxBrightness = -1;
	for(int i = 0; i < c.points.size(); i++){
		if(brightnessMatrix[c.points[i].x][c.points[i].y] > maxBrightness){
			maxBrightness = brightnessMatrix[c.points[i].x][c.points[i].y];
			return c.points[i];
		}
	}
	printf("Error, Couldn't find max brightness point\n");
}

doubleCoordinates calcClusterGravityCenter(int** brightnessMatrix, vector<coordinates> points){
	doubleCoordinates gravCenter;

	double brightSumX = 0.0, brightSumY = 0.0, brightSum = 0.0;


	for (int i = 0; i < points.size(); i++)
	{
		brightSumX += brightnessMatrix[points[i].x][points[i].y] * (points[i].x);
		brightSumY += brightnessMatrix[points[i].x][points[i].y] * (points[i].y);

		brightSum += brightnessMatrix[points[i].x][points[i].y];
	}

	gravCenter.x = brightSumX / brightSum;
	gravCenter.y = brightSumY / brightSum;
	return gravCenter;
}

vector<cluster> createClusters(int** brightnessMatrix){
	vector<cluster> clusters;
	coordinates curPoint;

	int maxBrightness = findMaxBrightness(brightnessMatrix);

	
	for (int y = 0; y < imageDimension.y; y++){
		for(int x = 0; x < imageDimension.x; x++){
			curPoint.x = x;
			curPoint.y = y;
			int neighbourIndex;

			if(brightnessMatrix[x][y] > 0 && pointHasNeighbour(curPoint, brightnessMatrix)){
				
				neighbourIndex = getClusterIndexOfNeighbour(curPoint, clusters);
				
				if(neighbourIndex == -1){
					cluster curCluster;
					curCluster.points.push_back(curPoint);
					clusters.push_back(curCluster);

				}else{
					clusters[neighbourIndex].points.push_back(curPoint);
				}
			}
		}
	}
	printf("%lu ROI angelegt\n", clusters.size());

	coordinates localMaxBrightnessPoint;
	
	for(int i = 0; i < clusters.size(); i++){
	 	localMaxBrightnessPoint = findMaxBrightnessPoint(clusters[i],brightnessMatrix);
		
	 	clusters[i].maxPoint.x = localMaxBrightnessPoint.x;
	 	clusters[i].maxPoint.y = localMaxBrightnessPoint.y;
		
		
		clusters[i].maxBrightness = brightnessMatrix[localMaxBrightnessPoint.x][localMaxBrightnessPoint.y];
		clusters[i].relBrightness = (double)clusters[i].maxBrightness / maxBrightness;
		
		clusters[i].size = calcClusterSize(clusters[i].relBrightness);
		clusters[i].gravCenter = calcClusterGravityCenter(brightnessMatrix, clusters[i].points);
		
		//printCluster(clusters[i]);
		
		writePointWithRadius(localMaxBrightnessPoint, clusters[i].size, outputMatrix);
	}
	writeBWMatrix(outputMatrix, header, "bild.bmp");
	return clusters;
}

double vectorNorm(coordinates a){
	return sqrt(pow(a.x,2) + pow(a.y,2));
}

coordinates searchCenterStar(vector<cluster> clusters){

        double tempDist = 0;
        coordinates diffCoordinate;
        
        //norm of imageCenter = maxDist
        double minDist = vectorNorm(imageCenter);
        coordinates minStar;
  
            for (vector<cluster>::iterator it = clusters.begin(); it != clusters.end(); it++){
                //wegen Nullpunktsverschiebung in die Bild Mitte:
                diffCoordinate.x = abs((*it).gravCenter.x - imageCenter.x);
                diffCoordinate.y = abs((*it).gravCenter.y - imageCenter.y);
                tempDist = vectorNorm(diffCoordinate);

                if(tempDist < minDist){
                        minDist = tempDist;
                        minStar.x = (*it).gravCenter.x;
                        minStar.y = (*it).gravCenter.y;
                }
                       
        }
        return minStar;
}

int main(){
	string filename = "bild.bmp";
	header = getImageHeader(filename);
	
	int headerSize = header.size();
	printf("Headersize: %d\n", headerSize);
	imageDimension = getPicDimensions(header);
	
	int** brightnessMatrix = createBrightnessMatrix(filename, headerSize);
	denoiseMatrix(brightnessMatrix, 18);
	writeGreyscaleMatrix(brightnessMatrix, header, filename);
	outputMatrix = createBinaryMatrix(imageDimension);

	vector<cluster> clusters = createClusters(brightnessMatrix);
	coordinates centerStar = searchCenterStar(clusters);
	printCoord(centerStar, "most central star: ");
	printf("cluster index of centerStar: %d\n", getClusterIndexOfPoint(centerStar, clusters)); 
	//Database *db = new Database("hip_red_1.txt", "tab1.txt", "tab2.txt", 0.025, 5.8e-6);

	return 0;
}