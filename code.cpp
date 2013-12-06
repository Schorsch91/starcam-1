#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "cluster.cpp"

using namespace std;

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
	printf("%d pixel below brightnessThreshold(%d) \n", cnt, brightnessThreshold);
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

				// if(isValidPixel(curPoint)){
				// 	printf("%d at ", brightnessMatrix[curPoint.x][curPoint.x]);
				// 	printCoord(curPoint);
				// }

				if(isValidPixel(curPoint) && 
					brightnessMatrix[curPoint.x][curPoint.x] >= localMaxBrightness &&
					!(i == 0 && j == 0)){
				
					maxBrightnessPoint.x = curPoint.x;
					maxBrightnessPoint.y = curPoint.y;

					foundMoreBrightness = true;

					localMaxBrightness = brightnessMatrix[curPoint.x][curPoint.y];
					printf("new maxBrightness %d\n", localMaxBrightness);
				}
			}
		}
		if(cnt > 10){
			maxBrightnessPoint.x = center.x;
			maxBrightnessPoint.y = center.y;
			printf("More than 10 tries, choosing center\n"); 
			break;
		}
	}while(!foundMoreBrightness);

	return maxBrightnessPoint;
}

void clearMatrixAroundPoint(int** matrix, coordinates point, int radius = 3){
	for(int i = -radius; i <= radius; i++){
			for (int j = -radius; j <= radius; j++)
			{
				matrix[point.x - i][point.y - j] = 0;
		}
	}
	printf("cleared with r=%d around", radius);
	printCoord(point);
}

double calcROISize(double relBrightness){
	if(relBrightness >= 0.75)
		return 9;
	else if(relBrightness >= 0.5)
		return 6;
	else if(relBrightness >= 0.25)
		return 4;
	else
		return 2;
}

vector<Cluster> createClusterArray(int** brightnessMatrix){
	vector<Cluster> clusters;
	bool** processedMatrix = createBinaryMatrix(imageDimension);
	coordinates curPoint;

	int maxBrightness = findMaxBrightnessAndDenoiseMatrix(brightnessMatrix, 25);
	int cnt = 0;
	printf("Max Brightness: %d\n", maxBrightness);

	for(int x = 0; x < imageDimension.x; x++){
		for (int y = 0; y < imageDimension.y; y++)
		{
			curPoint.x = x;
			curPoint.y = y;
			coordinates localMaxBrightnessPoint;
			int localMaxBrightness;
			double relBrightness;

			if(brightnessMatrix[x][y] > 0){
				printf("curBrightness: %d at ", brightnessMatrix[x][y]);
				printCoord(curPoint, "curPoint: ");
				
				localMaxBrightnessPoint = findLocalMaxBrightness(brightnessMatrix, curPoint, brightnessMatrix[x][y], 3);
				
				printCoord(localMaxBrightnessPoint, "localMaxBrightnessPoint: ");
				
				localMaxBrightness = brightnessMatrix[localMaxBrightnessPoint.x][localMaxBrightnessPoint.y];
				clearMatrixAroundPoint(brightnessMatrix, curPoint, 3);
				relBrightness = (double)localMaxBrightness / maxBrightness;
				

				printf("localMax: %d\n", localMaxBrightness);
				printf("relBrightness: %f\n\n\n", relBrightness);
				cnt++;
				
			}

		}
	}
	printf("FINISHED LOOP!\n");
	printf("%d cluster angelegt\n", cnt);
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
	printCoord(coord, "Dimension");
	return coord;
}

int main(){
	string filename = "bild.bmp";
	int headerSize = getHeaderSize(filename);
	printf("Headersize: %d\n", headerSize);
	imageDimension = getPicDimensions(filename);
	int** brightnessMatrix = createBrightnessMatrix(filename, headerSize);
	vector<Cluster> roi = createClusterArray(brightnessMatrix);
	return 0;
}