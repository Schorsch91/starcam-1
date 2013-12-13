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
	int size = 1; 
	int maxBrightness;
	double relBrightness;
	vector<coordinates> points;
};

ifstream inputPic;
ofstream outputPic;

coordinates imageDimension;

void writePixelWithValue(unsigned int val){
	outputPic.put(val);
	outputPic.put(val);
	outputPic.put(val);
}

void writeBlackPixel(){
	writePixelWithValue((unsigned int)0);
}

void writeWhitePixel(){
	writePixelWithValue((unsigned int)255);
}

string appendToFilename(string filename, string addition){
	int pos = filename.find_first_of(".");
	return filename.insert(pos, addition);
}

void writeBWImageToFile(string filename, int imageStart, bool** bwMatrix){
	outputPic.close();
	inputPic.close();

	string outputFilename = appendToFilename(filename, "BW");
	printf("imgStart: %d in: %s, out: %s\n", 
		imageStart, filename.c_str(), outputFilename.c_str());
	int cnt = 0;

	outputPic.open(outputFilename);
	inputPic.open(filename);
	inputPic.seekg(0,ios_base::beg);
	
	for(int i = 0; i < imageStart; i++){
		outputPic.put(inputPic.get());
	}
	for(int x = 0;x < imageDimension.x; x++){
		for(int y = 0;y < imageDimension.y; y++){
			if(bwMatrix[x][y]){
				writeWhitePixel();
				cnt++;
			} else{
				writeBlackPixel();
			}
		}
	}
	//printf("%d white pixels written\n", cnt);
	outputPic.close();
	inputPic.close();
}

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
	printf("Brightness %d (rel %f) at ", c.maxBrightness, c.relBrightness);
	printCoord(c.maxPoint);
	printf("Size %d around gravity center ", c.size);
	printCoord(c.gravCenter);
	printf("Contains %lu pixel\n\n", c.points.size());
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

bool isValidPixel(coordinates pixel, bool checkForValidPrinting = false){
	if(checkForValidPrinting){
		return !(pixel.x < 1 || pixel.x > (imageDimension.x - 1) || 
			pixel.y < 1 || pixel.y > (imageDimension.y - 1));
	} else{
		return !(pixel.x < 0 || pixel.x > imageDimension.x || 
			pixel.y < 0 || pixel.y > imageDimension.y);
	}
}

void createBrightnessMatrix(int** matrix, string filename, int headerSize){
	int count = 0, brightness = 0;

	inputPic.open(filename, fstream::binary);
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
				printf("%d at (%d|%d)\n", matrix[i][j], i, j);
			}
			brightness = 0;	
		}
	}
	printf("non-black pixels: %d\n", count);
	printf("black pixels: %d\n", (imageDimension.x * imageDimension.y) - count);
}

int findMaxBrightness(int** brightnessMatrix){
	int curMaxBrightness = 0;
	
	for(int x = 0; x < imageDimension.x; x++){
		for (int y = 0; y < imageDimension.y; y++)
		{
			if(brightnessMatrix[x][y] > curMaxBrightness){
				curMaxBrightness = brightnessMatrix[x][y];
			}
		}
	}
	return curMaxBrightness;
}

void denoiseMatrix(int** brightnessMatrix, int brightnessThreshold = 20){
	int cnt = 0;
	for(int x = 0; x < imageDimension.x; x++){
		for (int y = 0; y < imageDimension.y; y++)
		{
			if(brightnessMatrix[x][y] < brightnessThreshold){
				brightnessMatrix[x][y] = 0;
				cnt++;
			}
		}
	}
	printf("%d pixel below brightnessThreshold(%d) \n", cnt, brightnessThreshold);
	
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
		return 8;
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

void surroundPointWithBool(bool value, coordinates point, bool** matrix, int radius = 1){
	for (int i = -radius; i <= radius; i++){
		for (int j = -radius; j <= radius; j++){
			matrix[point.x + i][point.y + j] = value;
		}
	}	
}

void printNeighbours(int** matrix, coordinates point, int radius = 1){
	if(isValidPixel(point, true)){
		for (int i = -radius; i <= radius; i++){
			for (int j = -radius; j <= radius; j++){
				if(i == 0 && j == 0)
					printf("X ");
				else
					printf("%d ", matrix[point.x + i][point.y + j]);
			}
			printf("\n");
		}
	}
	printf("\n");
}

void writePointWithRadius(coordinates point, int radius, bool** matrix){
	switch(radius){
		case 1:
				matrix[point.x][point.y] = true;
				break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
				matrix[point.x][point.y] = true;
				//surroundPointWithBool(true, point, matrix, radius - 1);
				break;
		default:
				printf("Radius: %d Coord: ", radius);
				printCoord(point);
				printf("Error writing point to matrix!\n");
	}
}

bool coordAreEqual(coordinates a, coordinates b){
	return (a.x == b.x) && (a.y == b.y);
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
			if(coordAreEqual(clusters[i].points[j], leftCoord) || coordAreEqual(clusters[i].points[j], upCoord) ||
				coordAreEqual(clusters[i].points[j], upperLeftCoord) || coordAreEqual(clusters[i].points[j], upperRightCoord)){
				return i;
			}
		}
	}

	return -1;
}

vector<cluster> createClusterArray(int** brightnessMatrix){
	vector<cluster> clusters;
	coordinates curPoint;

	bool** outputMatrix = createBinaryMatrix(imageDimension);
	int maxBrightness = findMaxBrightness(brightnessMatrix);

	for(int x = 0; x < imageDimension.x; x++){
		for (int y = 0; y < imageDimension.y; y++)
		{
			curPoint.x = x;
			curPoint.y = y;
			int neighbourIndex;
			if(x == 861 && y == 124)
				printNeighbours(brightnessMatrix, curPoint, 2);
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
		
		outputMatrix[localMaxBrightnessPoint.x][localMaxBrightnessPoint.y] = true;

		// clusters[i].maxBrightness = brightnessMatrix[localMaxBrightnessPoint.x][localMaxBrightnessPoint.y];
		// clusters[i].relBrightness = (double)clusters[i].maxBrightness / maxBrightness;
		
		// clusters[i].size = calcClusterSize(clusters[i].relBrightness);
		//clusters[i].gravCenter = calcClusterGravityCenter(brightnessMatrix, clusters[i].points);
		
		//printCluster(clusters[i]);
		writePointWithRadius(localMaxBrightnessPoint, clusters[i].size, outputMatrix);
	}

	writeBWImageToFile("bild.BMP", 54, outputMatrix);

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
	string filename = "bild.BMP";
	int headerSize = getHeaderSize(filename);
	printf("Headersize: %d\n", headerSize);
	imageDimension = getPicDimensions(filename);
	int** brightnessMatrix = createIntegerMatrix(imageDimension);
	createBrightnessMatrix(brightnessMatrix, filename, headerSize);
	
	coordinates tempPoint;
	tempPoint.x = 861;
	tempPoint.y = 124;
	printNeighbours(brightnessMatrix, tempPoint, 2);
	
	vector<cluster> roi = createClusterArray(brightnessMatrix);

	return 0;
}