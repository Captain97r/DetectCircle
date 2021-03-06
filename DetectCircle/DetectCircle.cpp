// DetectCircle.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
using namespace cv;
using namespace std;



//void thresh_callback(int, void*);

int main()
{
	int thresh = 100;
	int max_thresh = 255;
	Mat gray;																						// Картинка в оттенках серого
																			
	vector<vector<Point>> contours;																	// Контейнер с координатами точек контуров
	vector<Vec4i> hierarchy;																		// Контейнер для хранения информации о связанных и родительских контуров для текущего
	Mat canny_output;																				// Массив для хранения границ, определенных с помощью детектора границ Кенни


	const char* filename = "img3.png";																// Имя файла картинки
    Mat src = imread( filename, IMREAD_COLOR );														// Массив с исходной картинкой


    if(src.empty()){
        printf(" Error opening image\n");															// Обработка ошибки открытия файла

		cin.get();
        return -1;
    }
    
    cvtColor(src, gray, COLOR_BGR2GRAY);															// В gray кладем исходную картинку в оттенках серого
    medianBlur(gray, gray, 5);																		// Применяем медианное сглаживание, т.е. удаляем шумы с картинки

    vector<Vec3f> circles;																			// Контейнер для координат найденных окружностей
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1, gray.rows/16, 100, 30, 1, 200);					// Стандартный метод библиотеки для определения окружностей, координаты всех найденных окружностей помещаются в контейнер circles
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);															// Координаты центра текущей окружности

        circle(src, center, 1, Scalar(0,100,100), 3, LINE_AA);										// Рисуем центр текущей окужности

        int radius = c[2];																			// Радиус окружности
        circle( src, center, radius, Scalar(0,0,255), 3, LINE_AA);									// Рисуем саму окружность с центом в точке center и радиусом radius

		cout << "CIRCLE " << i + 1 << ":   ";
		cout << "center: [" << c[0] << "; " << c[1] << "]; radius: " << c[2] << endl;				// Выод информации об окружности в стандартный поток ввода-вывода

    }
	
	Canny(gray, canny_output, thresh, thresh * 2, 3);												// Определение границ
	findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));	// Нахождение контуров 

	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);

	vector<vector<Point>> contours_poly(contours.size());											// Контейнер для хранения сглаженных контуров прямоугольников
	vector<Rect> boundRect(contours.size());														// Контейнер для хранения координат ограничивающих контуры прямоугольниов

	vector<Point2f> centers;																		// Координаты центров обнаруженных прямоугольников
	bool isExists = false;
	int c = 0;
	
	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);									// Сглаживание углов обнаруженных контуров
		boundRect[i] = boundingRect(Mat(contours_poly[i]));											// Определяем координаты ограничивающих контуры прямоугольников
	}

	for (size_t i = 0; i < contours_poly.size(); i++)
	{
		RotatedRect rect = minAreaRect(contours_poly[i]);											// Прямоугольник, описанный по контуру
		Size size = rect.size;
		double rectSize = size.height*size.width;													// Площадь прямоугольника, описанного по контуру
		double contourSize = contourArea(contours_poly[i]);
		if ((rectSize > 500) && ((rectSize + 200) > contourSize) && ((rectSize - 200) < contourSize))
		{
			for (size_t t = 0; t < centers.size(); t++)
			{
				if ((rect.center.x + 1 > centers[t].x) && (rect.center.x - 1 < centers[t].x) && (rect.center.y + 1 > centers[t].y) && (rect.center.y - 1 < centers[t].y))
				{
					isExists = true;																// Если координаты центра текущего прямоугольника совпадают с хотя бы одинм уже определенным прямоугольников, 
					break;																			// то пропустить такой прямоугольник (часто вокруг одногой фигуры отрисовываются несколько контуров)
				}
			}
			if (!isExists)																			// Если прямоугольник уникальный, то вывести инофрмацию о нем и обвести его местополжение
			{
				centers.push_back(rect.center);
				cout << "SQUARE " << ++c << ":   ";
				cout << "Contour area: " << contourArea(contours_poly[i]) << "; bounding rectangle area: " << rectSize << "; center: " << rect.center << endl;			// Вывод информации
				rectangle(src, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 3, 16, 0);																		// Обвод границ
				circle(src, rect.center, 1, Scalar(0, 100, 100), 3, 16);																								// Рисование центра прямоугольника
			}
			else isExists = false;
		}
	}

	for (size_t i = 0; i < centers.size(); i++)														// Последовательное определение расстояний между каждой фигурой:
	{
		for (size_t j = i + 1; j < centers.size(); j++)
		{
			float x_delta = pow((centers[j].x - centers[i].x), 2);
			float y_delta = pow((centers[j].y - centers[i].y), 2);
			float distance = sqrtf(x_delta + y_delta);
			cout << "Distance between square " << i + 1 << " and square " << j + 1 << " is " << distance << endl;
		}
	}

	for (size_t i = 0; i < circles.size(); i++)
	{
		for (size_t j = i + 1; j < circles.size(); j++)
		{
			float x_delta = pow((circles[j][0] - circles[i][0]), 2);
			float y_delta = pow((circles[j][1] - circles[i][1]), 2);
			float distance = sqrtf(x_delta + y_delta);
			cout << "Distance between circle " << i + 1 << " and circle " << j + 1 << " is " << distance << endl;
		}
	}
	for (size_t i = 0; i < circles.size(); i++)
	{
		for (size_t j = 0; j < centers.size(); j++)
		{
			float x_delta = pow((circles[i][0] - centers[j].x), 2);
			float y_delta = pow((circles[i][1] - centers[j].y), 2);
			float distance = sqrtf(x_delta + y_delta);
			cout << "Distance between circle " << i + 1 << " and square " << j + 1 << " is " << distance << endl;
		}
	}

	imshow("Detected geometry", src);																// Вывод картинки с отмеченными контурами всех определенных фигур

	waitKey(0);
	return(0);
}