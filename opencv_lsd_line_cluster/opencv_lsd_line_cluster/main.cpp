#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include <vector>
#include<iostream>
#include<string>
using namespace cv;
using namespace std;
#define lineSidesFeatures bayes
Mat _drawIm, _drawImBayes;
int image_width;
int image_height;



/*
class Vec2d
{
public:
double theta;
double r;
Vec2d(double theta, double r)
{
this->r = r;
this->theta = theta;
}

};

typedef struct Vec4d_
{
cv::Point2d p1;
cv::Point2d p2;
}Vec4d;
*/


//p[0] u1 p[1] v1
//p[2] u2 p[3] v2

Vec2d getLineEquationParas(Vec4d p)
{
	float k = (p[1] - p[3]) / (p[0] - p[2]);
	float y_intercept = p[1] - k*p[0];


	return Vec2d(k, y_intercept);
}



Vec2d getPolarLine(Vec4d p)
{
	if (fabs(p[0] - p[2]) < 1e-5)//��ֱֱ��
	{
		if (p[0] > 0)
			return Vec2d(p[0], 0);
		else
			return Vec2d(p[0], CV_PI);
	}

	if (fabs(p[1] - p[3]) < 1e-5) //ˮƽֱ��
	{
		if (p[1] > 0)
			return Vec2d(p[1], CV_PI / 2);
		else
			return Vec2d(p[1], 3 * CV_PI / 2);
	}

	float k = (p[1] - p[3]) / (p[0] - p[2]);
	float y_intercept = p[1] - k*p[0];

	float theta;

	if (k < 0 && y_intercept > 0)
		theta = atan(-1 / k);
	else if (k > 0 && y_intercept > 0)
		theta = CV_PI + atan(-1 / k);
	else if (k< 0 && y_intercept < 0)
		theta = CV_PI + atan(-1 / k);
	else if (k> 0 && y_intercept < 0)
		theta = 2 * CV_PI + atan(-1 / k);

	float _cos = cos(theta);
	float _sin = sin(theta);

	float r = p[0] * _cos + p[1] * _sin;

	return Vec2d(r, theta);
}

//��⵽������ֱ���߶����ü������ʾ��Ȼ����з��࣬ͬ���ֱ�߷�����ͬ�ı�ǩ�š�
//Ȼ�����ͬ��ǩ�ŵ��߶ζ�Ӧ�ļ�������м�Ȩƽ������Ϊ��Ӧֱ�ߡ� 

// vector<Vec2d> polarLines �Ǽ����������߶ζ�Ӧ�ļ������ʾ

bool getIndexWithPolarLine(vector<int>& _index, vector<Vec2d> polarLines, vector<Vec4f> filter)
{
	cv::Mat label_img = _drawIm.clone();
	int polar_num = polarLines.size();

	if (polar_num == 0)
	{
		return false;
	}

	_index.clear();
	_index.resize(polar_num);

	//��ʼ����ǩ��
	for (int i = 0; i < polar_num;i++)
		_index[i] = i;


	for (int i = 0; i < polar_num - 1;i++)
	{
		cv::Point top(filter[i][0], filter[i][1]);
		cv::Point bottom(filter[i][2], filter[i][3]);
		//	cv::line(label_img, top, bottom, cv::Scalar(0, 0, 255), 2);

		float minTheta = CV_PI;
		float minR = 50;
		Vec2d polar1 = polarLines[i];

		for (int j = i + 1; j < polar_num; j++)
		{

			Vec2d polar2 = polarLines[j];

			float dTheta = fabs(polar2[1] - polar1[1]);
			float dR = fabs(polar2[0] - polar1[0]);

			if (dTheta < minTheta)
				minTheta = dTheta;

			if (dR < minR)
				minR = dR;
			//ͬ��ֱ�߽Ƕ�������1.8�㣬����������8%
			if (dTheta < 2.5*CV_PI / 180 && dR < polar1[0] * 0.12)
			{
				_index[j] = _index[i];
				//	cv::line(label_img, Point2f(filter[j][0], filter[j][1]), Point2f(filter[j][2], filter[j][3]), cv::Scalar(255, 0, 0), 2);
			}

		}
	}
	return true;
}


static const int hdiv_table180[] = {
	0, 1044480, 522240, 348160, 261120, 208896, 174080, 149211,
	130560, 116053, 104448, 94953, 87040, 80345, 74606, 69632,
	65280, 61440, 58027, 54973, 52224, 49737, 47476, 45412,
	43520, 41779, 40172, 38684, 37303, 36017, 34816, 33693,
	32640, 31651, 30720, 29842, 29013, 28229, 27486, 26782,
	26112, 25475, 24869, 24290, 23738, 23211, 22706, 22223,
	21760, 21316, 20890, 20480, 20086, 19707, 19342, 18991,
	18651, 18324, 18008, 17703, 17408, 17123, 16846, 16579,
	16320, 16069, 15825, 15589, 15360, 15137, 14921, 14711,
	14507, 14308, 14115, 13926, 13743, 13565, 13391, 13221,
	13056, 12895, 12738, 12584, 12434, 12288, 12145, 12006,
	11869, 11736, 11605, 11478, 11353, 11231, 11111, 10995,
	10880, 10768, 10658, 10550, 10445, 10341, 10240, 10141,
	10043, 9947, 9854, 9761, 9671, 9582, 9495, 9410,
	9326, 9243, 9162, 9082, 9004, 8927, 8852, 8777,
	8704, 8632, 8561, 8492, 8423, 8356, 8290, 8224,
	8160, 8097, 8034, 7973, 7913, 7853, 7795, 7737,
	7680, 7624, 7569, 7514, 7461, 7408, 7355, 7304,
	7253, 7203, 7154, 7105, 7057, 7010, 6963, 6917,
	6872, 6827, 6782, 6739, 6695, 6653, 6611, 6569,
	6528, 6487, 6447, 6408, 6369, 6330, 6292, 6254,
	6217, 6180, 6144, 6108, 6073, 6037, 6003, 5968,
	5935, 5901, 5868, 5835, 5803, 5771, 5739, 5708,
	5677, 5646, 5615, 5585, 5556, 5526, 5497, 5468,
	5440, 5412, 5384, 5356, 5329, 5302, 5275, 5249,
	5222, 5196, 5171, 5145, 5120, 5095, 5070, 5046,
	5022, 4998, 4974, 4950, 4927, 4904, 4881, 4858,
	4836, 4813, 4791, 4769, 4748, 4726, 4705, 4684,
	4663, 4642, 4622, 4601, 4581, 4561, 4541, 4522,
	4502, 4483, 4464, 4445, 4426, 4407, 4389, 4370,
	4352, 4334, 4316, 4298, 4281, 4263, 4246, 4229,
	4212, 4195, 4178, 4161, 4145, 4128, 4112, 4096
};




// ֱ�������������������:������Աȶȣ�ɫ�ʵ�
// Vec6d  data: 
// data[0] u1, data[1] v1  first line point
// data[2] u2, data[3] v2  second line point
// data[4] line width
// data[5] line length
// ksize  ��������
#ifdef lineSidesFeatures
bool lineSidesFeature(Mat _input, Vec4d data, int ksize)
{

	const int hsv_shift = 12;

	//_input �����ͼ�����ݣ�Ϊ��ɫͼ
	Mat gray;
	if (_input.channels() == 3)
		cvtColor(_input, gray, CV_BGR2GRAY);
	else
		_input.copyTo(gray);


	//  Mat drawIm = _input.clone();

	float x1 = data[0];
	float y1 = data[1];
	float x2 = data[2];
	float y2 = data[3];

	//  line(_drawIm,Point2f(x1,y1),Point2f(x2,y2),Scalar(0,255,0));
	//  imshow("Sample line",drawIm);
	//  waitKey(10);

	//ֱ����������ĻҶ�ֵ
	vector<int> left_side;
	vector<int> right_side;

	//�Ҷ�ֵ��
	int left_sum = 0;
	int right_sum = 0;

	//ɫ���뱥�Ͷ�
	int left_H = 0;
	float left_S = 0.0;
	int right_H = 0;
	float right_S = 0.0;

	//��ɫ���ظ���
	int color_pix_num = 0;
	//��������
	int sample_num = 0;

	//��ֱֱ�ߣ�
	int vertical = 0;
	//ֱ��б�� �ؾ�
	float _k = 0;
	float _b = 0;

	//����ֱ�߱��ʽ
	if (fabs(x1 - x2) < 1e-2) // x = b;
	{
		vertical = 1;
		_b = x1;
	}
	else
	{
		_k = (y1 - y2) / (x1 - x2); //ֱ�߷��� kx + b = y;
		_b = y1 - _k*x1;
	}

	//  cout<<"vertical = "<<vertical<<endl;

	// fabs(_k) > 1,������ֱ����ˮƽ�ģ���Ӧͼ7�к�ɫ����
	// fabs(_k) < 1, ������ֱ���Ǵ�ֱ�ģ���Ӧͼ7����ɫ����

	int sample_line_type = fabs(_k) > 1 ? 1 : 0;

	//  cout<<"sample_line_type = "<< sample_line_type<<endl;

	//�趨���������㣬ȡ�߶ζ˵㣬������xֵ��С����yֵ��С������sample_line_type ��ֵ��
	//����ѭ��ȡ��һ����ʱ���Բ��ϵ���+1

	float u1, u2;
	int step = 1; // ����step = 1�����step = -1;��ô����ֵ������ֵ��
	if (vertical == 1)
	{
		if (y1 < y2)
		{
			u1 = y1;
			u2 = y2;
		}
		else
		{
			u1 = y2;
			u2 = y1;
		}
	}
	else
	{
		if (sample_line_type == 1)
		{
			if (y1 < y2)
			{
				u1 = y1;
				u2 = y2;
			}
			else
			{
				u1 = y2;
				u2 = y1;
			}
		}
		else
		{
			if (x1 < x2)
			{
				u1 = x1;
				u2 = x2;
			}
			else
			{
				u1 = x2;
				u2 = x1;
			}
		}
	}

	//  cout <<"step = "<<step <<endl;

	// ��ֱ�ߵ�һ���˵㿪ʼ���в������ö˵�Ҫô��ͼ������u�����Ҫô��ͼ������v���������Ϊstep
	// �õ�������󣬼�����õ㴹ֱ��ֱ�ߵķ��ߣ������ϵĵ���Ϊ�����㡣
	// �ڷ����ϲɼ������������Ϊ 2*ksize+ 1, �����ķ�������ֱ�ߵ�б�ʾ�������x������y���򣬲���Ϊ1��
	for (float u = u1; u <= u2; u += step)
	{
		float v0;

		float v1, v2;

		//����ֱ�ߵ�б����ؾ�
		float sk, sb;
		if (vertical == 1)
		{
			v0 = x1;
		}
		else
		{
			if (sample_line_type == 1)
			{
				v0 = (u - _b) / _k;
				sk = -1 / (1e-6 + _k);
				sb = u - sk*v0;
			}
			else
			{
				v0 = _k*u + _b;
				sk = -1 / (1e-6 + _k);

				sb = v0 - sk*u;
			}
		}

		v1 = v0 - ksize;
		v2 = v0 + ksize;

		//      cout<<"v1 = "<<v1<<", v2 = "<<v2<<endl;
		if (vertical == 1)
		{
			line(_drawIm, Point2f(v1, u), Point2f(v2, u), Scalar(0, 0, 255));
		}
		else
		{
			if (sample_line_type == 1)
			{
				line(_drawIm, Point2f(v1, sk*v1 + sb), Point2f(v2, sk*v2 + sb), Scalar(0, 0, 255));
			}
			else
				line(_drawIm, Point2f((v1 - sb) / sk, v1), Point2f((v2 - sb) / sk, v2), Scalar(255, 0, 0));
		}

		// sample_line_type = 1 ,��P0(v0,u)��ֱ���ϣ���ֱ�ڸ�ֱ�߲�����P0�����в�������x���򲽳�Ϊ1���в��������x = v1���յ�x =v2;
		// sample_line_tpye = 0, ��P0(u,v0)��ֱ���ϣ���ֱ�ڸ�ֱ�߲�����P0�����в�������y���򲽳�Ϊ1���в���, ���y = v1, �յ�y = v2��

		for (float v = v1; v <= v2; v += 1)
		{
			sample_num++;

			int x, y;
			if (vertical == 1) //��ֱ�߶�
			{
				x = (int)v;
				y = (int)u;
			}
			else
			{
				if (sample_line_type == 1) //��ˮƽ������,v����x�������
				{
					x = (int)v;
					y = (int)(sk*v + sb);
				}
				else //����ֱ�������� v����y�������
				{
					x = (int)((v - sb) / sk);
					y = (int)v;
				}
			}
			//��һ�����Ҫ��
			if (x < 0 || x > gray.cols - 1 || y < 0 || y > gray.rows - 1)
				continue;

			int nx = MAX(0, x);
			nx = min(nx, gray.cols - 1);

			int ny = MAX(0, y);
			ny = min(ny, gray.rows - 1);

			int val = gray.at<uchar>(ny, nx);
			Vec3b pixel = _input.at<Vec3b>(ny, nx);

			int b = pixel[0];
			int g = pixel[1];
			int r = pixel[2];

			int _max = MAX(b, MAX(g, r));
			int _min = MIN(b, MIN(g, r));


			int C = _max - _min;
			float S = 0;
			int H = 0;
			if (C > 10)
			{
				S = (float)C / _max;
				int vr = _max == r ? -1 : 0;
				int vg = _max == g ? -1 : 0;

				H = (vr & (g - b)) +
					(~vr & ((vg & (b - r + 2 * C)) + ((~vg) & (r - g + 4 * C))));
				H = (H * hdiv_table180[C] + (1 << (hsv_shift - 1))) >> hsv_shift;
				H += H < 0 ? 180 : 0;

				//ɫ���ж�ֱ���������Ƶ���ɫ
				if (S > 0.1 && H > 10)
					color_pix_num++;
			}

			if (vertical == 1)
			{
				if (nx > _b)
				{
					right_side.push_back(val);
					right_sum += val;

					right_H += H;
					right_S += S;
				}
				else
				{
					left_side.push_back(val);
					left_sum += val;

					left_H += H;
					left_S += S;
				}
			}
			else
			{
				float d = _k*nx + _b - ny;

				if (d > 0)
				{
					right_side.push_back(val);
					right_sum += val;

					right_H += H;
					right_S += S;
				}
				else
				{
					left_side.push_back(val);
					left_sum += val;

					left_H += H;
					left_S += S;
				}
			}

		} //v

	}//u

	int l_num = left_side.size();
	int r_num = right_side.size();
	//  cout << l_num <<" "<< r_num << endl;

	float left_mean = (float)(left_sum) / l_num;
	float right_mean = (float)(right_sum) / r_num;

	float left_H_mean = (float)(left_H) / l_num;
	float left_S_mean = (float)(left_S) / l_num;

	float right_H_mean = (float)(right_H) / r_num;
	float right_S_mean = (float)(right_S) / r_num;


	float left_var = 0, right_var = 0;
	for (int m = 0; m < l_num; m++)
	{
		left_var += (left_side[m] - left_mean)*(left_side[m] - left_mean);
	}
	if (l_num > 2)
		left_var = sqrtf(left_var) / (l_num - 1);

	for (int n = 0; n < r_num; n++)
	{
		right_var += (right_side[n] - right_mean)*(right_side[n] - right_mean);
	}
	if (r_num > 2)
		right_var = sqrtf(right_var) / (r_num - 1);


	cout << "���ȣ� " << left_mean << " " << right_mean << endl;
	cout << "���Ͷ� S��" << left_S_mean << " " << right_S_mean << " " << fabs(left_S_mean - right_S_mean) / (1e-5 + MAX(left_S_mean, right_S_mean)) << endl; // ɸѡֱ��������ɫ���� �ο�ֵ0.4
	cout << "���Ͷ� H��" << left_H_mean << " " << right_H_mean << " " << fabs(left_H_mean - right_H_mean) / (1e-5 + MAX(left_H_mean, right_H_mean)) << endl; // ɸѡֱ��������ɫ���� �ο�ֵ 0.15 
	cout << "����/��ֵ��" << left_var / left_mean << " " << right_var / right_mean << endl; //ɸѡƽ������
	cout << "************************************************************************* " << endl;
	//  imshow("Sample line",drawIm);
	//  waitKey(0);

	if (fabs(left_S_mean - right_S_mean) / (1e-5 + MAX(left_S_mean, right_S_mean)) < 0.4)
	{
		//���������˵,���֤��Ե����ɫ�������
		return false;
	}
	return true;
}
#endif

void convetAllCoordinateToPolarLine(string imagepath, vector<Vec2d> polarLines)
{


}

int getBigCoorX_Or_Y(int coordtype, int small_big_flag, Vec4f line)
{
	auto lamda_get_bigest = [coordtype, small_big_flag](auto p2)
	{
		if (coordtype == 0)//x��
		{
			int x1 = p2[0];
			int x2 = p2[2];
			int maxx = x1 > x2 ? x1 : x2;
			int minx = x1 > x2 ? x2 : x1;
			return small_big_flag == 0 ? minx : maxx;
		}
		else {
			int y1 = p2[1];
			int y2 = p2[3];
			int maxy = y1 > y2 ? y1 : y2;
			int miny = y1 > y2 ? y2 : y1;
			return small_big_flag == 0 ? miny : maxy;
		}
	};
	return lamda_get_bigest(line);
}

//�������֤�ı߱Ƚϳ������Խ���ɸѡ����ֱ��
double getLineLength(Vec4f line)
{
	auto lamda_get_line_length = [](auto p2)
	{

		int x1 = p2[0];
		int x2 = p2[2];


		int y1 = p2[1];
		int y2 = p2[3];

		double lineLength = (double)sqrt(pow((double(x1 - x2)) / image_width, 2) + pow((double(y1 - y2)) / image_height, 2));

		return lineLength;

	};
	return lamda_get_line_length(line);
}


//ɸѡ������ͬ��ǩ��ֱ�߽��о���,length_thresholdʱ���֤��Եֱ�ߵĹ�����ֵ
void cluster_line(vector<int>_index, vector<Vec4f>& m_linesVector, vector<Vec6d>& cluster_line, float length_threshold)
{
	vector<vector<int>> all_same_indexs;
	cv::Mat drawLines = _drawImBayes.clone();
	int coordtype;




	for (int i = 0;i < _index.size() - 1;i++)
	{
		int index_i = _index.at(i);
		vector<int> signal_same_index;
		vector<int>::iterator ret;
		vector<vector<int>>::iterator rets = all_same_indexs.begin();
		bool exist_ = false;
		for (;rets != all_same_indexs.end();rets++)
		{
			ret = std::find(rets->begin(), rets->end(), i);
			if (ret != rets->end())
			{
				exist_ = true;
				break;
			}
		}

		if (!exist_)
		{
			signal_same_index.push_back(i);
			for (int j = i + 1;j < _index.size();j++)
			{
				int index_j = _index.at(j);
				if (index_i == index_j)
				{
					signal_same_index.push_back(j);
				}
			}
		}

		all_same_indexs.push_back(signal_same_index);
	}

	int all_num_class = all_same_indexs.size();

	//�������еı�ǩ������ֱ�߾���
	for (int i = 0;i < all_num_class;i++)
	{
		vector<int> signal_same_index = all_same_indexs.at(i);
		double sum_line_k = 0.0f;
		double sum_line_y_intercept = 0.0f;
		double minx = 9999999.0f;
		double miny = 9999999.0f;
		double maxx = -9999999.0f;
		double maxy = -9999999.0f;

		int signal_same_index_size = signal_same_index.size();
		if (signal_same_index_size == 0)
			continue;
		double sum_length = 0.0f;
		for (int j = 0;j < signal_same_index_size;j++)
		{
			//��¼����ֱ���е��߶��ܺ͵�k��b
			int line_index = signal_same_index[j];
			Vec2d line_equation_parameters = getLineEquationParas(m_linesVector.at(line_index));
			sum_line_k += line_equation_parameters[0];
			sum_line_y_intercept += line_equation_parameters[1];
			//sum_length += getLineLength(m_linesVector.at(line_index));



			//��¼����ֱ���е��߶��е�minx,maxx,miny,maxy

			double line_minx = getBigCoorX_Or_Y(0, 0, m_linesVector.at(line_index));

			double line_maxx = getBigCoorX_Or_Y(0, 1, m_linesVector.at(line_index));

			if (minx > line_minx)
			{
				minx = line_minx;
			}
			if (line_maxx > maxx)
			{
				maxx = line_maxx;
			}

			/*
			maxy = getBigCoorX_Or_Y(1, 0, m_linesVector.at(line_index));
			miny = getBigCoorX_Or_Y(1, 1, m_linesVector.at(line_index));
			*/


		}
		/*
		if (sum_length < length_threshold)
		{
		continue;
		}
		*/

		double ave_lines_k = sum_line_k / signal_same_index_size;
		double ave_lines_y_intercept = sum_line_y_intercept / signal_same_index_size;

		//����ave_lines_k��ave_lines_y_intercept���Ƴ�ֱ��(���ݲ�ͬ���߶ζ�)

		/*
		double top_y = 0.0f;
		double bottom_y = (double)image_height;

		//�ٶ����е�ֱ�߶�����top_y��bottom_y����ֱ��
		double top_x = (top_y - ave_lines_y_intercept) / ave_lines_k;
		double bottom_x = (bottom_y - ave_lines_y_intercept) / ave_lines_k;
		*/

		miny = int((minx)*ave_lines_k + ave_lines_y_intercept);
		maxy = int((maxx)*ave_lines_k + ave_lines_y_intercept);

		cv::Point top(minx, miny);
		cv::Point bottom(maxx, maxy);
		Vec6d signal_cluster_line_info;
		Vec4d signal_cluster_line;
		signal_cluster_line[0] = minx;
		signal_cluster_line[1] = miny;
		signal_cluster_line[2] = maxx;
		signal_cluster_line[3] = maxy;

		signal_cluster_line_info[0] = minx;
		signal_cluster_line_info[1] = miny;
		signal_cluster_line_info[2] = maxx;
		signal_cluster_line_info[3] = maxy;



		signal_cluster_line_info[4] = getLineLength(signal_cluster_line);
		signal_cluster_line_info[5] = 15;

		cluster_line.push_back(signal_cluster_line_info);
		cv::line(drawLines, top, bottom, cv::Scalar(255, 0, 0), 2);

	}

	imwrite("./drawLines.jpg", drawLines);

}

void getLineByLsd(string imagepath)
{
	//ԭʼͼƬ
	_drawIm = imread(imagepath);
	_drawImBayes = _drawIm.clone();
	cv::Mat _srcIm = _drawIm.clone();
	cv::Mat last_filter_img = _drawIm.clone();
	cv::Mat hsv_filter_img = _drawIm.clone();

	image_width = _drawIm.cols;
	image_height = _drawIm.rows;

	cv::Mat lsdSegments = _drawIm.clone();

	//ֱ�߾���ת��Ϊ������ļ���
	vector<Vec2d> polarLines;

	//���ݼ�����ֱ�ߵı�ǩ
	vector<int> _index;

	Mat gray;
	if (_drawIm.channels() == 3)
		cvtColor(_drawIm, gray, CV_BGR2GRAY);
	else
		_drawIm.copyTo(gray);

	//LSDֱ�߼��
	vector<cv::Vec4f> m_linesVector, filter_length_linesVector;
	cv::Ptr<cv::LineSegmentDetector> ls = cv::createLineSegmentDetector(cv::LineSegmentDetectorModes::LSD_REFINE_ADV);

	ls->detect(gray, m_linesVector);

	//ֱ�ߵĳ��ȹ���
	vector<cv::Vec4f>::iterator ite = m_linesVector.begin();
	/*
	for (int i = 0;i < m_linesVector.size();i++)
	{
	double line_length = getLineLength(m_linesVector[i]);
	if (line_length < 0.25)
	{
	m_linesVector.erase(ite + i);
	}
	}*/
	int count_index = 0;

	for (ite;ite != m_linesVector.end();ite++)
	{
		double line_length = getLineLength(*ite);

		if (line_length > 0.08)
		{
			filter_length_linesVector.push_back(*ite);
		}
	}


	ls->drawSegments(lsdSegments, filter_length_linesVector);


	//������ɫhsv�Ĺ���


#define hsv_first yes

#ifdef hsv_first
	vector<Vec4f> hsv_filter_lines;
	vector<Vec6d> last_filter_lines;

	//������ɫ����H,S,V����ֱ��
	for (int i = 0;i < filter_length_linesVector.size();i++)
	{
		bool is_pass_filter = lineSidesFeature(_srcIm, filter_length_linesVector[i], 1);
		if (is_pass_filter)
		{
			hsv_filter_lines.push_back(filter_length_linesVector[i]);
			cv::Point top(filter_length_linesVector[i][0], filter_length_linesVector[i][1]);
			cv::Point bottom(filter_length_linesVector[i][2], filter_length_linesVector[i][3]);
			cv::line(hsv_filter_img, top, bottom, cv::Scalar(255, 0, 0), 2);

		}
	}

	//���е�ֱ�����ھ���hsv_filter_lines��
	for (int i = 0;i < hsv_filter_lines.size();i++)
	{
		polarLines.push_back(getPolarLine(hsv_filter_lines[i]));
	}

	bool ret = getIndexWithPolarLine(_index, polarLines, hsv_filter_lines);

	if (ret == false)
	{
		cout << "getIndexWithPolarLine(_index, polarLines) error";
		return;
	}

	//ɸѡ������ͬ��ǩ��ֱ�߽��о���

	//��ȡ������ֱ�߶�
	vector<Vec6d>cluster_lines;

	cluster_line(_index, hsv_filter_lines, cluster_lines, 0.5);


#endif

	/*
	//���е�ֱ�����ھ���m_linesVector��
	for (int i = 0;i < filter_length_linesVector.size();i++)
	{
	polarLines.push_back(getPolarLine(filter_length_linesVector[i]));
	}
	bool ret = getIndexWithPolarLine(_index, polarLines,filter_length_linesVector);

	if (ret == false)
	{
	cout << "getIndexWithPolarLine(_index, polarLines) error";
	return;
	}
	//ɸѡ������ͬ��ǩ��ֱ�߽��о���

	//��ȡ������ֱ�߶�
	vector<Vec6d>cluster_lines;
	cluster_line(_index, filter_length_linesVector,cluster_lines,0.5);

	vector<Vec6d> last_filter_lines;

	//������ɫ����H,S,V����ֱ��

	for (int i = 0;i < cluster_lines.size();i++)
	{
	bool is_pass_filter = lineSidesFeature(_srcIm, cluster_lines[i], 1);
	if(true)
	{
	last_filter_lines.push_back(cluster_lines[i]);
	cv::Point top(cluster_lines[i][0], cluster_lines[i][1]);
	cv::Point bottom(cluster_lines[i][2], cluster_lines[i][3]);
	cv::line(last_filter_img,top,bottom, cv::Scalar(255, 0, 0), 2);
	}
	}
	*/
}

void Cpp_11_Example()
{
	Vec4f beyes(1, 2, 3, 4);
	auto it = beyes;
	cout << it[0];
	cout << it[1];
	cout << it[2];
	cout << it[3];
}

//������ϵ��ֱ�ߵ��󽻵㷽����������Ҫע�����㣺
//���ȣ�����ֱ�߲���ƽ�еģ���Σ�ֱ�ߵĽ�����ͼ��Χ�ڡ�
Point2f polarLinesCorss(Vec2d l0, Vec2d l1,Size sz)
{

	int w = sz.width;
	int h = sz.height;

	float r0 = l0[0];
	float theta0 = l0[1];

	float _cos0 = cos(theta0);
	float _sin0 = sin(theta0);

	float r1 = l1[0];
	float theta1 = l1[1];

	float _cos1 = cos(theta1);
	float _sin1 = sin(theta1);

	if (fabs(_cos0*_sin1 - _sin0*_cos1) < 1e-5) //����ƽ�е�ֱ��
		return Point2f(0, 0);

	float y = (r0*_cos1 - r1*_cos0) / (_sin0*_cos1 - _cos0*_sin1);
	float x = (r0*_sin1 - r1*_sin0) / (_cos0*_sin1 - _cos1*_sin0);

	if (x > -w / 2 && x < w / 2 && y > -h / 2 && y < h / 2)
		return Point2f(x + w / 2, h / 2 - y);
	else
		return Point2f(0, 0);
}

// �����Ͻǵ�Ϊ�����ʱ������
static void sortPoints(vector<Point2f> & points)
{
	vector<Point2f> minXpoints;
	vector<Point2f> maxXpoints;

	minXpoints.push_back(points[0]);
	minXpoints.push_back(points[1]);

	maxXpoints.push_back(points[2]);
	maxXpoints.push_back(points[3]);

	for (int i = 0; i< 2; i++)
	{
		float x = minXpoints[i].x;
		if (x > maxXpoints[0].x)
		{
			if (x >= maxXpoints[1].x)
			{
				(maxXpoints[0].x > maxXpoints[1].x) ? swap(maxXpoints[1], minXpoints[i]) : swap(maxXpoints[0], minXpoints[i]);
				continue;
			}

			if (x < maxXpoints[1].x)
			{
				swap(maxXpoints[0], minXpoints[i]);
				continue;
			}
		}

		if (x <= maxXpoints[0].x)
			if (x > maxXpoints[1].x)
			{
				swap(minXpoints[i], maxXpoints[1]);

			}

	}

	if (minXpoints[0].y > minXpoints[1].y)
	{
		points[0] = minXpoints[1];
		points[1] = minXpoints[0];
	}
	else
	{
		points[0] = minXpoints[0];
		points[1] = minXpoints[1];
	}

	if (maxXpoints[0].y > maxXpoints[1].y)
	{
		points[2] = maxXpoints[0];
		points[3] = maxXpoints[1];
	}
	else
	{
		points[2] = maxXpoints[1];
		points[3] = maxXpoints[0];
	}

}





void main()
{
	string imagepath = "idcard.jpg";
	getLineByLsd(imagepath);
	return;
}