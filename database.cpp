#include "database.h"

double Database::calcStarCamDistance(coordinates3d c){
	return sqrt(pow(c.x,2) + pow(c.y,2) + pow(c.z,2));
}

double Database::calcDistanceBetweenStars(coordinates3d a, coordinates3d b){
	return sqrt(pow(a.x-b.x,2) + pow(a.y-b.y,2) + pow(a.z-b.z,2));
}

double Database::calcAlphaAngle(double distStarNeighbour, double distCamStar, double distCamNeighbourStar){
	return acos((distStarNeighbour*distStarNeighbour-distCamStar*distCamStar-distCamNeighbourStar*distCamNeighbourStar) / (-2*distCamStar*distCamNeighbourStar));	
}

double Database::calcBetaAngle(coordinates3d c1, coordinates3d c2, double dist1, double dist2){
	double dist_a1_a2_squared = (c1.x-c2.x)*(c1.x-c2.x)+(c1.y-c2.y)*(c1.y-c2.y)+(c1.z-c2.z)*(c1.z-c2.z);
	return acos((dist_a1_a2_squared-dist1*dist1-dist2*dist2) / (-2*dist1*dist2));
}


Database::Database(string inputfile, string filenameTable1, string filenameTable2, double foc_len, double pix_size)
{
	focalLength = foc_len;
	pixelSize = pix_size;
	
	printf("create db..\n");
	//falls datenbanken noch nicht erstellt, diese erzeugen
	//andernfalls textdateien einlesen
	ifstream file1;
	file1.open(filenameTable1);
	if (file1.good()) {
		printf("table1 already imported - read from %s\n", filenameTable1.c_str());
		tab1 data;
		while (file1.good()) {
			file1 >> data.id >> data.x >> data.y >> data.z >> data.mag;
			dataTable1.push_back(data);
		}
		file1.close();
	} else {
		file1.close();
		importInputfile(inputfile);
		generateTable1 (filenameTable1);
	}
	
	ifstream file2;
	file2.open(filenameTable2);
	if (file2.good()) {
		printf("table2 already imported - read from %s\n", filenameTable2.c_str());
		//bereits generierte daten der tabelle einlesen
		tab2 data2;
		while (file2.good()) {
			file2 >> data2.id1 >> data2.id2 >> data2.id3 >> data2.alpha1 >> data2.alpha2 >> data2.beta;
			dataTable2.push_back(data2);
		}
		file2.close();
	} else {
		file2.close();
		generateTable2 (filenameTable2);
	}
	printf("db ready for action\n");
}

void Database::importInputfile(string filename) {

	printf("read from file %s\n", filename.c_str());
	ifstream file;
	file.open (filename);
	
	double ra;
	double dec;
	double mag;
	
	in data;
	int counter = 1;
	while(file) {
		file >> ra >> dec >> mag;
		
		if (mag <= MAGNITUDE_THRESHOLD) {
			data.id = counter;
			data.dec = dec;
			data.ra = ra;
			data.mag = mag;
			input_data.push_back(data);
			counter++;
		}
	}

	file.close();
}

/*
	import and convert raw data from given database into file and c++ vector
*/
void Database::generateTable1(string filename) {

	printf("create first table in %s \n", filename.c_str());
	printf("format: [ID, X, Y, Z, Mag]\n");
	
	double x;
	double y;
	double z;
	tab1 data;
	
	ofstream out;
	out.open(filename);

	for (vector<in>::iterator it = input_data.begin(); it != input_data.end(); it++) {
		x = cos((*it).dec) * cos ((*it).ra);
		y = cos((*it).dec) * sin ((*it).ra);
		z = (*it).mag * sin((*it).dec);
		
		data.id = (*it).id;
		data.x = x;
		data.y = y;
		data.z = z;
		data.mag = (*it).mag;
		
		dataTable1.push_back(data);
		
		out << data.id << " " << x << " " << y << " " << z << " " << data.mag << " ";
	}
	out.close();
}

/*
	generate second table from the first, converted one
*/
void Database::generateTable2(string filename) {
	
	printf("create second table in %s \n", filename.c_str());
	printf("format: [ID1, ID2, ID3, a1, a2, beta]\n");

	double a1;
	double a2;
	double beta;
	tab2 data;

	double distCamStar; //vektor von cam zum iterierenden stern
	double distCamNeighbourStar; //vektor von cam zu nachbarsternen des it. stern
	double distStarNeighbour; //vektor zwischen it. stern und nachbarsternen

	//datei für tabelle 2
	ofstream out;
	out.open(filename);

	double alpha;
	double dist_a1_it; //vektor vom iterierenden stern zu stern unter a1
	double dist_a2_it; //vektor vom iterierenden stern zu stern unter a2
	double dist_a1_a2_squared; //vektor vom stern unter a1 zum stern unter a2 zum quadrat
	coordinates3d c1; //koordinaten zum stern unter a1
	coordinates3d c2; //koordinaten zum stern unter a2
	tab1 data2; //daten zum stern unter a1
	tab1 data3; //daten zum stern unter a2

	coordinates3d curStar;
	coordinates3d curNeighbourStar;

	int cnt = 0, size = dataTable1.size();

	for (vector<tab1>::iterator it = dataTable1.begin(); it != dataTable1.end(); it++) {
		printf("%.2f%% \r", 100 * (double)cnt/size);
		
		//calc 2 smallest alpha
		a1 = 1000;
		a2 = 1000;
		curStar.x = (*it).x;
		curStar.y = (*it).y;
		curStar.z = (*it).z;
		

		distCamStar = calcStarCamDistance(curStar); //sqrt (pow((*it).x,2) + pow((*it).y,2) + pow((*it).z,2));
		//alle nachbarsterne durchlaufen
		for (vector<tab1>::iterator neighbourStar = it+1; neighbourStar!=dataTable1.end(); neighbourStar++) {
			
			curNeighbourStar.x = (*neighbourStar).x;
			curNeighbourStar.y = (*neighbourStar).y;
			curNeighbourStar.z = (*neighbourStar).z;


			distCamNeighbourStar = calcStarCamDistance(curStar); 
			//sqrt (pow((*neighbourStar).x,2) + pow((*neighbourStar).y,2) + pow((*neighbourStar).z,2));
			distStarNeighbour = calcDistanceBetweenStars(curNeighbourStar, curStar); 
			//sqrt (pow((*neighbourStar).x-(*it).x,2) + pow((*neighbourStar).y-(*it).y,2) + pow((*neighbourStar).z-(*it).z,2));
			
			alpha = calcAlphaAngle(distStarNeighbour, distCamStar, distCamNeighbourStar);

			if (alpha < a1) {
				a1 = alpha;
				data2 = (*neighbourStar);
				dist_a1_it = distStarNeighbour;
				c1.x = (*neighbourStar).x;
				c1.y = (*neighbourStar).y;
				c1.z = (*neighbourStar).z;
			} else if (alpha < a2) {
				a2 = alpha;
				data3 = (*neighbourStar);
				dist_a2_it = distStarNeighbour;
				c2.x = (*neighbourStar).x;
				c2.y = (*neighbourStar).y;
				c2.z = (*neighbourStar).z;
			}
			
		}

		//beta berechnen mit kosinussatz
		beta = calcBetaAngle(c1, c2, dist_a1_it, dist_a2_it);
		//zwei kleinsten nachbarn zu diesem stern in datenbank aufnehmen
		data.id1 = (*it).id;
		data.id2 = data2.id;
		data.id3 = data3.id;
		data.alpha1 = a1;
		data.alpha2 = a2;
		data.beta = beta;
		//daten in vector übernehmen
		dataTable2.push_back(data);
		//daten in datei schreiben
		out << data.id1 << " " << data.id2 << " " << data.id3 << " " << data.alpha1 << " " << data.alpha2 << " " << data.beta << " ";
		cnt++;
	}

	out.close();
}

bool Database::find_triple (angle_triple key, tab2 *result) {
	//durchsuche dataTable2 nach key
	(*result).id1=0;
	(*result).id2=0;
	(*result).id3=0;
	(*result).alpha1=0;
	(*result).alpha2=0;
	(*result).beta=0;

	double inaccuracy = 0.01;
	double variance; //square of the standard deviation
	double varianceThreshold = 1000;

	for (vector<tab2>::iterator it = dataTable2.begin(); it != dataTable2.end(); it++) {
		if ((*it).alpha1 < key.alpha1+inaccuracy && (*it).alpha1 > key.alpha1-inaccuracy)
			if ((*it).alpha2 < key.alpha2+inaccuracy && (*it).alpha2 > key.alpha2-inaccuracy)
				if ((*it).beta < key.beta+inaccuracy && (*it).beta > key.beta-inaccuracy) {
					//abweichung vom gesuchten wert berechnen
					variance = ((*it).alpha1-key.alpha1)*((*it).alpha1-key.alpha1)+((*it).alpha2-key.alpha2)*((*it).alpha2-key.alpha2)+((*it).beta-key.beta)*((*it).beta-key.beta);
					if (variance < varianceThreshold) {
						*result = *it;
						varianceThreshold = variance;
					}
				}	
	}
	
	if ((*result).beta != 0)
		return true;
	return false;
}

vector3d Database::coordsOfStarWithID(double id) {
	vector3d star;
	
	for (vector<tab1>::iterator it = dataTable1.begin(); it!=dataTable1.end(); it++) {
		if ((*it).id == id) {
			star.x = (*it).x;
			star.y = (*it).y;
			star.z = (*it).z;
			break;
		}
	}
	
	return star;
}
