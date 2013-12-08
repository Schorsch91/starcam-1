#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

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
	int size; 
	int brightness;
	double relBrightness;
};

ifstream inputPic;
ofstream outputPic;

coordinates imageDimension;

int getHeaderSize(string filename){
	inputPic.close();
	inputPic.open(filename);
	for(int a=0; a<10; a++)
    {
        inputPic.get();
    }
    int val = inputPic.get();
    inputPic.close();
    return val;
}

/* 
	returns file size in kb
*/
int getFileSize(string filename){
	long begin,end;
	// inputPic.close();
	inputPic.open(filename);
  	begin = inputPic.tellg();
  	inputPic.seekg(0,ios::end); 
  	end = inputPic.tellg();
  	// inputPic.close();
  	return (end-begin);
}

void printCoord(coordinates c, string s = ""){
	printf("%s(%d|%d)\n",s.c_str(), c.x, c.y);
}

void printCoord(doubleCoordinates c, string s = ""){
	printf("%s(%f|%f)\n",s.c_str(), c.x, c.y);
}

void printCluster(cluster c, string s = ""){

}

bool** createBinaryMatrix(coordinates dimension){
	bool** m = new bool*[dimension.x];
	for(int i = 0; i < dimension.x; i++){
		m[i] = new bool[dimension.y];
	}
	return m;
}

int** createIntegerMatrix(coordinates dimension){
	int** m = new int*[dimension.x];
	for(int i = 0; i < dimension.x; i++){
		m[i] = new int[dimension.y];
	}
	return m;
}

bool isValidPixel(coordinates pixel){
	return !(pixel.x < 0 || pixel.x > imageDimension.x || 
			pixel.y < 0 || pixel.y > imageDimension.y);
}

int** createBrightnessMatrix(string filename, int headerSize){
	int** matrix = createIntegerMatrix(imageDimension);
	int count = 0, brightness = 0;
	inputPic.open(filename);
	inputPic.seekg(0,ios_base::beg);
	for (int i=0; i < headerSize; i++){
		inputPic.get();
	}
	for(int i = 0; i < imageDimension.x; i++){
		for (int j = 0; j < imageDimension.y; j++)
		{
			brightness += inputPic.get();
			brightness += inputPic.get();
			brightness += inputPic.get();
			matrix[i][j] = brightness / 3;
			if(brightness > 0){
				count++;
			}
			brightness = 0;	
		}
	}
	printf("non-black pixels: %d\n", count);
	return matrix;
}

int findMaxBrightnessAndDenoiseMatrix(int** brightnessMatrix, int brightnessThreshold = 20){
	int curMaxBrightness = 0;
	int cnt = 0;
	
	for(int x = 0; x < imageDimension.x; x++){
		for (int y = 0; y < imageDimension.y; y++)
		{
			if(brightnessMatrix[x][y] < brightnessThreshold){
				brightnessMatrix[x][y] = 0;
				cnt++;
			}
			else if(brightnessMatrix[x][y] > curMaxBrightness){
				curMaxBrightness = brightnessMatrix[x][y];
			}
		}
	}
	//printf("%d pixel below brightnessThreshold(%d) \n", cnt, brightnessThreshold);
	return curMaxBrightness;
}

coordinates findLocalMaxBrightness(int** brightnessMatrix, coordinates center, int localMaxBrightness, int radius = 3){
	coordinates curPoint, maxBrightnessPoint;
	
	bool foundMoreBrightness = false;
	int cnt = 0;
	do{
		cnt++;
		for(int i = -radius; i <= radius; i++){
			for (int j = -radius; j <= radius; j++)
			{
				curPoint.x = center.x + i;
				curPoint.y = center.y + j;

				if(isValidPixel(curPoint) && 
					brightnessMatrix[curPoint.x][curPoint.x] >= localMaxBrightness &&
					!(i == 0 && j == 0)){
				
					maxBrightnessPoint.x = curPoint.x;
					maxBrightnessPoint.y = curPoint.y;

					foundMoreBrightness = true;

					localMaxBrightness = brightnessMatrix[curPoint.x][curPoint.y];
				}
			}
		}
		if(cnt > 10){
			maxBrightnessPoint.x = center.x;
			maxBrightnessPoint.y = center.y;
			
			break;
		}
	}while(!foundMoreBrightness);

	return maxBrightnessPoint;
}

void clearMatrixAroundPoint(int** matrix, coordinates point, int radius = 3){
	for(int i = -radius; i <= radius; i++){
			for (int j = -radius; j <= radius; j++)
			{
				matrix[point.x + i][point.y + j] = 0;
		}
	}
}

int calcClusterSize(double relBrightness){
	if(relBrightness >= 0.75)
		return 9;
	else if(relBrightness >= 0.5)
		return 6;
	else if(relBrightness >= 0.25)
		return 4;
	else
		return 2;
}

int** copyMatrix(int** matrix, coordinates dimension){
	int** returnMatrix = createIntegerMatrix(dimension);

	for (int i = 0; i < dimension.x; i++)
	{
		for (int j = 0; j < dimension.y; j++)
		{
			returnMatrix[i][j] = matrix[i][j];
		}
	}
	return returnMatrix;
}

doubleCoordinates calcClusterGravityCenter(int** brightnessMatrix, coordinates point, int size){
	doubleCoordinates gravCenter;

	double brightSumX = 0.0, brightSumY = 0.0, brightSum = 0.0;


	for (int i = -size; i <= size; ++i)
	{
		for (int j = -size; j <= size; j++)
		{
			brightSumX += brightnessMatrix[point.x + i][point.y + j] * (point.x + i);
			brightSumY += brightnessMatrix[point.x + i][point.y + j] * (point.y + j);
		
			brightSum += brightnessMatrix[point.x + i][point.y + j];
		}
	}

	gravCenter.x = brightSumX / brightSum;
	gravCenter.y = brightSumY / brightSum;
	return gravCenter;
}

vector<cluster> createClusterArray(int** brightnessMatrix, int maxBrightness){
	vector<cluster> clusters;
	coordinates curPoint;

	int** workMatrix = copyMatrix(brightnessMatrix, imageDimension);

	//printf("Max Brightness: %d\n", maxBrightness);

	for(int x = 0; x < imageDimension.x; x++){
		for (int y = 0; y < imageDimension.y; y++)
		{
			curPoint.x = x;
			curPoint.y = y;
			coordinates localMaxBrightnessPoint;
			int localMaxBrightness;
			
			if(workMatrix[x][y] > 0){
				localMaxBrightnessPoint = findLocalMaxBrightness(workMatrix, curPoint, workMatrix[x][y]);
				localMaxBrightness = workMatrix[localMaxBrightnessPoint.x][localMaxBrightnessPoint.y];
				clearMatrixAroundPoint(workMatrix, curPoint);

				cluster curCluster;

				curCluster.maxPoint.x = localMaxBrightnessPoint.x;
				curCluster.maxPoint.x = localMaxBrightnessPoint.x;
				curCluster.brightness = localMaxBrightness;
				curCluster.relBrightness = localMaxBrightness / maxBrightness;
				curCluster.size = calcClusterSize(curCluster.relBrightness);
				curCluster.gravCenter = calcClusterGravityCenter(brightnessMatrix, localMaxBrightnessPoint, curCluster.size);

				clusters.push_back(curCluster);
			}

		}
	}
	printf("%lu ROI angelegt\n", clusters.size());
	return clusters;
}

coordinates getPicDimensions(string filename){
	int width;
	int height;
	int c;
	

	inputPic.open(filename);
	inputPic.seekg(0, ios_base::beg);
	for(int i=0; i < 18; i++){
		inputPic.get();
	}
	c = inputPic.get();
	width = c;
	c = inputPic.get();
	width = c << 8 | width;
	c = inputPic.get();
	width = c << 16 | width;
	c = inputPic.get();
	width = c << 24 | width;
	
	c = inputPic.get();
	height = c;
	c = inputPic.get();
	height = c << 8 | height;
	c = inputPic.get();
	height = c << 16 | height;
	c = inputPic.get();
	height = c << 24 | height;
	inputPic.close();
	coordinates coord;
	coord.x = width;
	coord.y = height;
	printCoord(coord, "Imagedimension: ");
	return coord;
}

int main(){
	string filename = "bild.bmp";
	int headerSize = getHeaderSize(filename);
	printf("Headersize: %d\n", headerSize);
	imageDimension = getPicDimensions(filename);
	printf("\n");
	int** brightnessMatrix = createBrightnessMatrix(filename, headerSize);
	int maxBrightness = findMaxBrightnessAndDenoiseMatrix(brightnessMatrix, 25);
	vector<cluster> roi = createClusterArray(brightnessMatrix, maxBrightness);
	return 0;
}