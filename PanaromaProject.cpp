#include<iostream>
#include<climits>
#include<cmath>
#include<ctime>
#include<map>
#include<vector>
#include<deque>
#include<opencv2/highgui/highgui.hpp>
#include<conio.h>

#define timeBound   30
#define alphaBeta   true
#define cutOff  true
#define inRange(x, y) ( (x >= 0) && (x < 10) && (y >= 0) && (y < 10) )
#define loop2(itr1, itr2, lim1, lim2) for(; itr1 < lim1; itr1++)  for(; itr2 < lim2; itr2++)
#define displayMat(matPr)  for(int itr1=0; itr1<10; itr1++)    {for(int itr2=0; itr2<10; itr2++)    if(matPr[itr1][itr2] == -1)     cout << "x"; else if(matPr[itr1][itr2] == 1) cout << "o";    else if(matPr[itr1][itr2] == 2)    cout << "c"; else   cout << ".";     cout<<endl;}
#define checkMap(mappy, val)    (mappy.find(val)) != mappy.end()

using namespace std;
using namespace cv;

clock_t clockStart;
long long int cnt, cnt2, cntStates;
bool contTurn = true;
int level;
int cntMoves;

//Variables used for GUI only
Mat bat;

int mX, mY, xTrk, yTrk;
int board[10][10] = {
	0,0,0,1,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	1,0,0,0,0,0,0,0,0,1,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	2,0,0,0,0,0,0,0,0,2,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,2,0,0,2,0,0,0 };
bool shwTrack = false, moveFirst = true, moveSecond = false, AI_Move = false, aniMov1 = false, aniMov2;
int ax1, ax2, ay1, ay2, ax3, ay3;
vector<vector<int> > refV(10, vector<int>(10, 0));

//GUI related variables declaration ends here

#define checkTime() ((double)(clock() - clockStart)/ CLOCKS_PER_SEC)

struct dir {
	bool dir1, dir2, dir3, dir4, dir5, dir6, dir7, dir8;
};

class node {
public:
	map<float, vector<int> > arr;
	map<int, node> treeRecord;
	map<int, bool> stateChk;

	void markNodes(bool order, int n = 20)
	{
		if (!order) {
			//cout<<"\n----------------------------"<<endl;
			map<float, vector<int> > ::iterator it = arr.begin();
			for (int i = 0; it != arr.end(); it++) {
				//if(i < n)
				//cout<<endl<<it->first<<" : ";
				if (i < n)
					while (it->second.empty() == false) {
						//cout<<it->second.back()<<" ";
						stateChk[it->second.front()] = true;
						it->second.pop_back();
						i++;
					}
				it->second.clear();
			}
		}
		else if (order) {
			//cout<<"\n++++++++++++++++++++++++++++++"<<endl;
			map<float, vector<int> > ::iterator it = arr.end();
			it--;
			for (int i = 0; it != arr.begin(); it--) {
				//if(i < n)
				//cout<<endl<<it->first<<" : ";
				if (i < n)
					while (it->second.empty() == false) {
						//cout<<it->second.back()<<" ";
						stateChk[it->second.back()] = true;
						it->second.pop_back();
						i++;
					}
				it->second.clear();
			}
		}
		//cout << endl;
	}
	void display() {
		cout << "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
		for (int i = 0; i<2000; i++)
			if (stateChk[i] == true)
				cout << i << " ";
		cout << endl;
	}

}dfNode;

class state {
public:

	//int mat[10][10];
	vector<vector<int> > mat;

	struct pos {
		int x, y;
	}position[2][4];

	class move {
	public:
		int x1, y1;
		int x2, y2;
		int f1, f2;

		void set(int x1, int y1, int x2, int y2, int f1, int f2) {
			this->x1 = x1;
			this->y1 = y1;
			this->x2 = x2;
			this->y2 = y2;
			this->f1 = f1;
			this->f2 = f2;
		}
	}finalMove, moveTmp;

	class moveGenerator {        //To generate the next possbile moves available
	public:
		int x, y;
		int xOriginal, yOriginal;
		int dirX, dirY;

		void set(int x, int y) {
			this->x = x;
			this->y = y;
			xOriginal = x;
			yOriginal = y;
			dirX = dirY = -1;
		}

		inline bool generate(vector<vector<int> >& mat) {
			if (inRange(dirX + x, dirY + y) && mat[dirY + y][dirX + x] == 0) {
				x += dirX;
				y += dirY;
			}
			else {
				x = xOriginal;
				y = yOriginal;
				if (dirX == -1 && dirY == -1)  dirX = 0;
				else if (dirX == 0 && dirY == -1)  dirX = 1;
				else if (dirX == 1 && dirY == -1)  dirY = 0;
				else if (dirX == 1 && dirY == 0)  dirY = 1;
				else if (dirX == 1 && dirY == 1)  dirX = 0;
				else if (dirX == 0 && dirY == 1)  dirX = -1;
				else if (dirX == -1 && dirY == 1)  dirY = 0;
				else  return false;
				if (inRange(dirX + x, dirY + y) && mat[dirY + y][dirX + x] == 0) {
					x += dirX;
					y += dirY;
				}
				else  return generate(mat);
			}
			return true;
		}
	};

	void initialize(vector<vector<int> > mat) {
		int idx[2];//To store the index of both bots to be feeded in
		idx[1] = idx[0] = 0;
		this->mat.resize(10, vector<int>(10));
		for (int i = 0; i<10; i++)
			for (int j = 0; j<10; j++) {
				this->mat[i][j] = mat[i][j];
				int v = mat[i][j];
				if (v != 0 && v != -1)
				{
					v--;
					position[v][idx[v]].y = i;
					position[v][idx[v]].x = j;
					idx[v]++;
				}
			}
	}

	class stPoint {
	public:
		int x, y;
		int flag;
		int cnt;
	};

	class matPoint {
	public:
		int own = -1;
		int steps = 1000;
		int dir[3] = { 0, 0, 0 };
	};

	int calcConnectedRegion(int x, int y, int own, vector<vector<matPoint> > &matTmp, vector<vector<bool> > &refV) {
		int count = 0;
		int f = 2;
		for (int i = -f; i <= f; i++)
			for (int j = -f; j <= f; j++) {
				if (i != 0 || j != 0) {
					if (inRange(y + i, x + j) && matTmp[y + i][x + j].own == own && refV[y + i][x + j] == false) {
						refV[y + i][x + j] = true;
						count = calcConnectedRegion(x + j, y + i, own, matTmp, refV) + 1;
					}
				}
			}
		return count;
	}

	float calcRegion(bool dis = false) {
		int cnt_ = 0, cnt__ = 0;
		matPoint matPt__;
		matPt__.own = -1;
		matPt__.steps = 1000;
		vector<vector<matPoint> > matTmp(10, vector<matPoint>(10, matPt__));
		deque<stPoint> buffer;
		for (int i1 = 0; i1 <= 1; i1++)
			for (int i2 = 0; i2 <= 3; i2++)
			{
				stPoint stTmp;
				stTmp.x = position[i1][i2].x;
				stTmp.y = position[i1][i2].y;
				if (i1 == 0)  stTmp.flag = 0;
				else    stTmp.flag = 1;
				stTmp.cnt = 0;
				buffer.push_back(stTmp);
			}
		while (buffer.empty() == false)
		{
			int pX, pY;
			stPoint stTmp = buffer.front();
			buffer.pop_front();
			pX = stTmp.x;
			pY = stTmp.y;
			int dir = 1;
			int dirX, dirY, x, y, cell = 0;
			x = pX;
			y = pY;
			dirX = dirY = -1;
			if (stTmp.flag <= 1 && matTmp[y][x].own == 2) {
				//cout << "SKIPPED\n";
				//cout << "OWNED : " << stTmp.flag << endl;
				continue;//Checks if this cell still belongs to the first element or not;
			}
			/*int cntH = 0, cntV = 0;
			cout << "OWNED : " << stTmp.flag << endl;
			cout << "Y : " << y << " X : "<< x<< endl;
			for(int i = 0; i < 10; i++){
			for(int j = 0; j < 10; j++){
			if( i == y && j == x)   cout << "&";
			else if(matTmp[i][j].own == 0){
			cntH++;
			cout << "+";
			}
			else if(matTmp[i][j].own == 1){
			cntV++;
			cout << "-";
			}
			else if(matTmp[i][j].own == 2)    cout << "o";
			else    cout << ".";
			cout << " ";
			}
			cout << endl;
			}
			cout << "H : "<< cntH << "V : "<< cntV;
			cout << "CNT1: "<<cnt_<<" CNT2: "<<cnt__ << endl;
			cnt_ = cnt__ = 0;
			int tmp__;
			cin >> tmp__;*/
			while (1)
			{
				cnt_++;
				if (inRange(dirX + x, dirY + y) && (matTmp[dirY + y][dirX + x].dir[stTmp.flag] & dir) == 0 && mat[dirY + y][dirX + x] == 0) {
					x += dirX;
					y += dirY;
				}
				else {
					x = pX;
					y = pY;
					if (dirX == -1 && dirY == -1)  dirX = 0, dir = 2;
					else if (dirX == 0 && dirY == -1)  dirX = 1, dir = 4;
					else if (dirX == 1 && dirY == -1)  dirY = 0, dir = 8;
					else if (dirX == 1 && dirY == 0)  dirY = 1, dir = 1;
					else if (dirX == 1 && dirY == 1)  dirX = 0, dir = 2;
					else if (dirX == 0 && dirY == 1)  dirX = -1, dir = 4;
					else if (dirX == -1 && dirY == 1)  dirY = 0, dir = 8;
					else  break;
					if (inRange(dirX + x, dirY + y) && (matTmp[dirY + y][dirX + x].dir[stTmp.flag] & dir) == 0 && mat[dirY + y][dirX + x] == 0) {
						x += dirX;
						y += dirY;
					}
					else    continue;
				}
				cnt__++;
				matPoint matPt_ = matTmp[y][x];
				stPoint stTmp_ = stTmp;
				stTmp_.x = x;
				stTmp_.y = y;
				if (stTmp.flag == 0) {
					stTmp_.cnt++;
					if (matPt_.own == -1 && mat[y][x] == 0) {
						matPt_.steps = stTmp.cnt + 1;
						matPt_.own = 0;
						buffer.push_back(stTmp_);
					}
					matPt_.dir[0] |= dir;
				}
				else if (stTmp.flag == 1) {
					//cout << matPt_.steps << " " << stTmp.cnt << endl;
					bool flag_ = false;
					if (matPt_.steps >= stTmp.cnt + 1 && matPt_.own == 0)
						matPt_.own = 2;
					else if (matPt_.own == -1)    matPt_.own = 1, flag_ = true;
					matPt_.dir[0] |= dir;
					matPt_.dir[1] |= dir;
					stTmp_.cnt++;
					if (flag_) {
						matPt_.steps = stTmp.cnt + 1;
						buffer.push_back(stTmp_);
					}
				}
				matTmp[y][x] = matPt_;
			}
		}

		/*int cntH = 0, cntV = 0;
		cout << endl;
		for(int i = 0; i < 10; i++){
		for(int j = 0; j < 10; j++){
		if(matTmp[i][j].own == 0){
		cntH++;
		cout << "+";
		}
		else if(matTmp[i][j].own == 1){
		cntV++;
		cout << "-";
		}
		else if(matTmp[i][j].own == 2)   cout << "o";
		else    cout << ".";
		cout << " ";
		}
		cout << endl;
		}*/
		//cout << "H: "<<cntH << "V: "<< cntV<< endl;
		//cout << "CNT1: " << cnt_ << " CNT2: " << cnt__ << endl;

		dis = false;
		int cntH = 0, cntV = 0;
		vector<vector<bool> >refV(10, vector<bool>(10, false));
		int scr[2] = { 0, 0 };
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				int own = matTmp[i][j].own;
				if (dis) {
					if (own == 0)    cout << "+";
					else if (own == 1)   cout << "-";
					else if (own == 2)   cout << "o";
					else    cout << ".";
				}
				if (own <= 1) {
					if (own == 0)    cntH++;
					else if (own == 1)   cntV++;
					if (refV[i][j] == false) {
						refV[i][j] = true;
						int val = calcConnectedRegion(i, j, own, matTmp, refV) + 1;
						if (val > scr[own]) scr[own] = val;
					}
				}
			}
			if (dis) cout << endl;
		}
		if (dis) {
			cout << "HERO: " << scr[0] << "  Vellion: " << scr[1] << endl;
			cout << "cntH: " << cntH << " cntV: " << cntV << endl;
		}
		float d1 = scr[0] / (float)(100 - 8 - cntMoves);
		float d2 = scr[1] / (float)(100 - 8 - cntMoves);
		if (dis) {
			cout << scr[0] << " " << d1 << " " << 100 - 8 - cntMoves << endl;
			cout << scr[1] << " " << d2 << " " << 100 - 8 - cntMoves << endl;
		}
		return scr[0] * 4 /** (d1 > 0.4 ? 1 : -1)*/ - scr[1] * 4 /** (d2 > 0.35 ? 1 : -1)*/ + cntH * 1 - cntV * 1;
	}

	float evaluate(bool dis = false) {
		float finalValue = 0;
		//vector<vector<int> > matRef(10, vector<int>(10, 0));
		//To check the one move direction cells available
		for (int i = 0; i<2; i++)
			for (int j = 0; j<4; j++)
			{
				int pX, pY;
				pX = position[i][j].x;
				pY = position[i][j].y;
				int dirX, dirY, x, y, cell = 0;
				x = pX;
				y = pY;
				dirX = dirY = -1;
				while (1)
				{
					if (inRange(dirX + x, dirY + y) && mat[dirY + y][dirX + x] == 0) {// && ((mat[dirY + y][dirX + x] & (i + 1)) == 0)) {
						x += dirX;
						y += dirY;
					}
					else {
						x = pX;
						y = pY;
						if (dirX == -1 && dirY == -1)  dirX = 0;
						else if (dirX == 0 && dirY == -1)  dirX = 1;
						else if (dirX == 1 && dirY == -1)  dirY = 0;
						else if (dirX == 1 && dirY == 0)  dirY = 1;
						else if (dirX == 1 && dirY == 1)  dirX = 0;
						else if (dirX == 0 && dirY == 1)  dirX = -1;
						else if (dirX == -1 && dirY == 1)  dirY = 0;
						else  break;
						if (inRange(dirX + x, dirY + y) && mat[dirY + y][dirX + x] == 0) {// && mat[dirY + y][dirX + x] & (i + 1)) {
							x += dirX;
							y += dirY;
						}
						else    continue;
					}
					cell++;
					//matRef[y][x] |= i + 1;
				}
				if (i == 0)  finalValue += cell * 4;
				else    finalValue -= cell * 3;
			}
		if (cntMoves > 10)
			finalValue += calcRegion(dis);
		return finalValue;
	}

	float decideMove(int pCode, int depth, bool isMaximizer, node &enlist, int alpha = INT_MIN, int beta = INT_MAX, bool topMostLevel = false) {

		//evaluate(true);int tmp;cin>>tmp;
		/*int i = 0;
		while(checkTime() < timeBound)
		evaluate(), i++;
		cout << i << endl;
		int tmp;cin>>tmp;*/
		float bestPoint = INT_MIN;
		map<float, bool> mappy;
		if (isMaximizer == false)    bestPoint = INT_MAX;
		int pCodeTmp = pCode;

		if (checkTime() > timeBound)
		{
			contTurn = false;
			if (isMaximizer)     return alpha;
			else    return beta;
		}

		int stateCnt = 0;
		for (int i = 0; i<4; i++) {

			int pX = position[pCode - 1][i].x;
			int pY = position[pCode - 1][i].y;
			moveGenerator moveAmazon;
			moveAmazon.set(pX, pY);
			while (moveAmazon.generate(mat)) {  //Iterates all possible moves the selected amazon
				moveGenerator fire;
				fire.set(moveAmazon.x, moveAmazon.y);
				mat[pY][pX] = 0;
				mat[moveAmazon.y][moveAmazon.x] = pCode;
				position[pCode - 1][i].x = moveAmazon.x;
				position[pCode - 1][i].y = moveAmazon.y;

				while (fire.generate(mat)) {

					//To check if state is marked or not
					mat[fire.y][fire.x] = -1;
					stateCnt++;
					float val = 0;
					val = evaluate();
					if (cutOff) {
						int i = stateCnt;
						if (depth == 1) {
							enlist.arr[val].push_back(stateCnt);
						}
						else if (enlist.stateChk[i] == false) {
							mat[fire.y][fire.x] = 0;
							continue;
						}
						cntStates++;
						if (depth > 1 && 0) {
							cout << val << "< - * -------\n";
							if (isMaximizer) cout << "++++++++++++++++++++\n";
							else    cout << "----------------------\n";
							displayMat(mat);
							cout << "\n\n";
							cout << val << endl;
							//cout << evaluate(true);
						}
					}
					if (checkTime() > timeBound)
					{
						contTurn = false;
						mat[fire.y][fire.x] = 0;
						mat[pY][pX] = pCode;
						mat[moveAmazon.y][moveAmazon.x] = 0;
						position[pCodeTmp - 1][i].x = pX;
						position[pCodeTmp - 1][i].y = pY;
						return (isMaximizer ? INT_MIN : INT_MAX);
					}
					node enlistNxt = enlist;
					if (cutOff) {
						if (depth == 2)
							enlist.treeRecord[stateCnt] = dfNode;
						if (depth == 1)     enlistNxt = enlist;
						else    enlistNxt = enlist.treeRecord[stateCnt];
					}

					if (isMaximizer)     //Maximizer layer
					{
						float valTmp = 0;
						if (depth != 1)
							valTmp = decideMove(pCode % 2 + 1, depth - 1, (isMaximizer ? false : true), enlistNxt, alpha, beta);
						else    valTmp = val, cnt++;
						if (valTmp > bestPoint) {
							bestPoint = valTmp;
							if (topMostLevel)//To register the best move available
							{
								moveTmp.set(pX, pY, moveAmazon.x, moveAmazon.y, fire.x, fire.y);
							}
						}
						if (bestPoint > alpha)  alpha = bestPoint;
						if (alpha >= beta && alphaBeta)
						{
							//cout<<"Beta CutOff;";
							mat[fire.y][fire.x] = 0;
							mat[pY][pX] = pCode;
							mat[moveAmazon.y][moveAmazon.x] = 0;
							position[pCodeTmp - 1][i].x = pX;
							position[pCodeTmp - 1][i].y = pY;
							return bestPoint;
						}
					}
					if (isMaximizer == false)    //Minimizer layer
					{
						float valTmp = 0;
						if (depth != 1)
							valTmp = decideMove(pCode % 2 + 1, depth - 1, (isMaximizer ? false : true), enlistNxt, alpha, beta);
						else    valTmp = val, cnt++;
						if (valTmp < bestPoint)  bestPoint = valTmp;
						if (bestPoint < beta)   beta = bestPoint;
						if (beta <= alpha && alphaBeta)
						{
							//cout<<"Alpha CutOff;";
							mat[fire.y][fire.x] = 0;
							mat[pY][pX] = pCode;
							mat[moveAmazon.y][moveAmazon.x] = 0;
							position[pCodeTmp - 1][i].x = pX;
							position[pCodeTmp - 1][i].y = pY;
							return bestPoint;
						}
					}
					if (depth > 1)
						enlist.treeRecord[stateCnt] = enlistNxt;
					mat[fire.y][fire.x] = 0;
				}
				mat[pY][pX] = pCode;
				mat[moveAmazon.y][moveAmazon.x] = 0;
				position[pCodeTmp - 1][i].x = pX;
				position[pCodeTmp - 1][i].y = pY;
			}
		}
		if (depth == 1 && cutOff)
		{
			enlist.markNodes(isMaximizer ? true : false);//, (50)/pow(2, level + 1 - depth));
														 //enlist.display();
		}
		return bestPoint;
	}
};

void box(int x, int y, int sz, int r, int g, int b, float f1, Mat &mat) {
	float f2 = 1 - f1;
	for (int i = 0; i < sz; i++)
		for (int j = 0; j < sz; j++)
			if (i + x < mat.cols && j + y < mat.rows && i + x >= 0 && j + y >= 0) {
				Vec3b pix = mat.at<Vec3b>(j + y, i + x);
				pix.val[0] = pix.val[0] * f1 + b*f2;
				pix.val[1] = pix.val[1] * f1 + g*f2;
				pix.val[2] = pix.val[2] * f1 + r*f2;
				mat.at<Vec3b>(j + y, i + x) = pix;
			}
}

void printBat(float x, float y, int r, int g, int b, Mat &mat, bool raw = true) {
	if (raw) {
		x = x * 80 + 0 * x;
		y = y * 80 + 12;
	}
	for (int i = 0; i < bat.cols; i++)
		for (int j = 0; j < bat.rows; j++)
			if (i + x >= 0 && i + x < mat.cols && y + j >= 0 && y + j < mat.rows) {
				Vec3b pix = bat.at<Vec3b>(j, i);
				if (pix.val[2] > 0) {
					Vec3b pix;
					pix.val[2] = b;
					pix.val[1] = g;
					pix.val[0] = r;
					mat.at<Vec3b>(j + y, i + x) = pix;
				}
			}
}

void callBackFunc(int event, int x, int y, int flags, void* userdata)
{
	mX = x;
	mY = y;
	if (event == EVENT_LBUTTONDOWN)
	{
		if (board[y / 80][x / 80] == 2) {
			shwTrack = true;
			xTrk = x / 80;
			yTrk = y / 80;
		}
		if (refV[y / 80][x / 80] == 1) {
			if (moveFirst) {
				aniMov1 = true;
				ax1 = xTrk;
				ax2 = x / 80;
				ay1 = yTrk;
				ay2 = y / 80;
				xTrk = x / 80;
				yTrk = y / 80;
				//shwTrack = false;
				moveFirst = false;
				moveSecond = true;
				shwTrack = false;
			}
			else {
				shwTrack = false;
				moveFirst = true;
				ay3 = y / 80;
				ax3 = x / 80;
				aniMov2 = true;
			}
		}
	}
	else if (event == EVENT_MOUSEMOVE)
	{
		mX = x;
		mY = y;
	}
}

int main() {
	cout << "At any point press 'R' to restart the game. To exit press 0 or esc.";
begin:
	int boardTmp[10][10] = {
		0,0,0,1,0,0,1,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		1,0,0,0,0,0,0,0,0,1,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		2,0,0,0,0,0,0,0,0,2,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,2,0,0,2,0,0,0 };
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
			board[i][j] = boardTmp[i][j];
	shwTrack = false, moveFirst = true, moveSecond = false, AI_Move = false;
	refV.clear();
	refV.resize(10, vector<int>(10, 0));
	bat = imread("vampi.png");
	Mat base = imread("bgk.jpeg");
	namedWindow("Game Of Amazons", 1);
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
			if ((i + j) % 2 == 0)
				box(i * 80, j * 80, 80, 255, 100, 0, 0.3, base);
			else
				box(i * 80, j * 80, 80, 250, 150, 50, 0.3, base);
	setMouseCallback("Game Of Amazons", callBackFunc, NULL);
	while (1) {

		if (AI_Move) {
			//cout << "AI move Requested\n";
			vector<vector<int> > mat(10, vector<int>(10));
			for (int i = 0; i<10; i++)
				for (int j = 0; j<10; j++) {
					mat[i][j] = board[i][j];
					if (mat[i][j] == -1) cntMoves++;
				}
			state stBegin;
			stBegin.initialize(mat);
			int cntFinal = 0;
			node enlist;
			clockStart = clock();
			for (int i = 1; i<3 && checkTime()<timeBound; i++)
			{
				cout << "Iterating...\n";
				stBegin.decideMove(1, i, true, enlist, INT_MIN, INT_MAX, true);
				if (contTurn)
				{
					stBegin.finalMove = stBegin.moveTmp;
					cntFinal = cnt;
					//cout<<"\n i : "<<i<<" time: "<<checkTime()<<" N: "<<cnt<<endl;
					level++;
				}
				cnt = 0;
			}
			aniMov1 = aniMov2 = true;
			ax1 = stBegin.finalMove.x1;
			ay1 = stBegin.finalMove.y1;
			ax2 = stBegin.finalMove.x2;
			ay2 = stBegin.finalMove.y2;
			ax3 = stBegin.finalMove.f1;
			ay3 = stBegin.finalMove.f2;
			/*board[stBegin.finalMove.y1][stBegin.finalMove.x1] = 0;
			board[stBegin.finalMove.y2][stBegin.finalMove.x2] = 1;
			board[stBegin.finalMove.f2][stBegin.finalMove.f1] = -1;*/
			//AI_Move = false;
		}
		Mat fin = base.clone();

		//Prints track
		refV.clear();
		refV.resize(10, vector<int>(10, 0));
		if (shwTrack) {
			for (int xDir = -1; xDir <= 1; xDir++)
				for (int yDir = -1; yDir <= 1; yDir++) {
					int x = xTrk;
					int y = yTrk;
					while (x + xDir >= 0 && x + xDir < 10
						&& y + yDir >= 0 && y + yDir < 10
						&& board[y + yDir][x + xDir] == 0) {
						box((x + xDir) * 80 + 5, (y + yDir) * 80 + 5, 70, 50, 20, 200, 0.6, fin);
						refV[y + yDir][x + xDir] = 1;
						x += xDir;
						y += yDir;
					}
				}
		}

		//Prints Current Cell
		if(!aniMov1 && !aniMov2)
			box(mX / 80 * 80 + 5, mY / 80 * 80 + 5, 70, 50, 255, 50, 0.2, fin);

		//Prints All Vampires
		for (int i = 0; i < 10; i++)
			for (int j = 0; j < 10; j++)
				if (board[i][j] == 1)
					printBat(j, i, 50, 50, 50, fin);
				else if (board[i][j] == 2)
					printBat(j, i, 250, 250, 250, fin);
				else if (board[i][j] == -1) {
					box(j * 80 + 5, i * 80 + 5, 70, 50, 50, 50, 0.1, fin);
					if (i == mY / 80 && j == mX / 80)
						box(mX / 80 * 80 + 5, mY / 80 * 80 + 5, 70, 50, 255, 50, 0.5, fin);
				}
				//To do the animation
				while (aniMov1) {
					float dx = ax2 - ax1, dy = ay2 - ay1;
					dx /= 100;
					dy /= 100;
					float xAni = ax1, yAni = ay1;
					Mat matAniRef = fin.clone();
					box(xAni * 80, yAni * 80, 80, 255, 100, 0, 0.3, matAniRef);
					//cout << "Beginning Animation\n" <<;
					while (abs(xAni - ax2) > 0.1 || abs(yAni - ay2) > 0.1) {
						Mat matAni = matAniRef.clone();
						printBat(xAni, yAni, 50, 50, 150, matAni, 1);
						xAni += dx;
						yAni += dy;
						waitKey(10);
						imshow("Game Of Amazons", matAni);
						if (aniMov2)
							fin = matAni.clone();
					}
					board[ay2][ax2] = (AI_Move ? 1 : 2);
					board[ay1][ax1] = 0;
					aniMov1 = false;
					if(!AI_Move)
						shwTrack = true;
					//fin = matAniRef.clone();
					//imshow("test", fin);
				}
				while (aniMov2) {					
					Mat matAniRef = fin.clone();
					float dr = 255 - 50, dg = 100 - 50, db = 0 - 50, di = 0.3 - 0.1;
					dr /= -20;
					dg /= -20;
					db /= -20;
					di /= -20;
					float r = 255, g = 100, b = 0, intensity = 0.3;
					for (int i = 0; i < 20; i++) {
						Mat matAni = matAniRef.clone();
						box(ax3 * 80 + 5, ay3 * 80 + 5, 70, r, g, b, intensity, matAni);
						r += dr;
						g += dg;
						b += db;
						intensity += di;
						waitKey(20);
						imshow("Game Of Amazons", matAni);
						fin = matAni.clone();
					}
					aniMov2 = 0;
					board[ay3][ax3] = -1;
					AI_Move = !AI_Move;
				}

		//To check if anyone wins or not
		int cnt1 = 0, cnt2 = 0;
		for (int i = 0; i < 10; i++)
			for (int j = 0; j < 10; j++)
				if (board[i][j] >= 1)
					for (int m = -1; m <= 1; m++)
						for (int n = -1; n <= 1; n++)
							if ((m != 0 || n != 0) && i + m >= 0 && i + m < 10 && j + n >= 0 && j + n < 10 && board[i + m][j + n] == 0) {
								if (board[i][j] == 1)
									cnt1++;
								else
									cnt2++;
								break;
							}

		Mat won = imread("won.bmp");
		Mat loss = imread("loss.bmp");
		int flag = 0;
		if (cnt1 == 0 && cnt2 == 0) {
			if (AI_Move) {
				cout << "You WON!!!!!!!!!!!!!!!!!";
				flag = 1;
			}
			else {
				cout << "AI WON!!!!!!!!!!!!!!!!!!";
				flag = 2;
			}
		}
		if (!cnt1 && cnt2) {
			cout << "You WON!!!!!!!!!!!!!!!!!!!!!";
			flag = 1;
		}
		if (cnt1 && !cnt2) {
			cout << "AI WON!!!!!!!!!!!!!!!!!!!!!!";
			flag = 2;
		}
		if (flag == 1) {
			for (int i = 0; i < won.rows; i++)
				for (int j = 0; j < won.cols; j++) {
					Vec3b pix = won.at<Vec3b>(i, j);
					if (pix[0] != 255 || pix[1] != 255 || pix[2] != 255)
						fin.at<Vec3b>(i + 80, j + 180) = pix;
				}
		}
		else if(flag == 2)
		{
			for (int i = 0; i < loss.rows; i++)
				for (int j = 0; j < loss.cols; j++) {
					Vec3b pix = loss.at<Vec3b>(i, j);
					if (pix[0] != 255 || pix[1] != 255 || pix[2] != 255)
						fin.at<Vec3b>(i + 80, j + 180) = pix;
				}
		}
		imshow("Game Of Amazons", fin);
		int key = waitKey(1);
		key %= 256;
		if (key == '0' || key == 27)
			return 0;
		else if (key == 'R' || key == 'r')
			goto begin;
		if (flag)
			break;
	}

	char ch;
	while (1)
	{
		if (_kbhit())
			ch = _getch();
		int key = waitKey(1);
		key %= 256;
		if (key == '0' || key == 27)
			return 0;
		else if (key == 'R' || key == 'r')
			goto begin;
	}
	
	if (ch == 'r' || ch == 'R')
		goto begin;

	return 0;
}
