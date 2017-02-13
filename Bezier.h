#ifndef BEZIER_H_
#define BEZIER_H_

#include <cmath>
#include <vector>

int factorial(int n) {
	int product = 1;
	for (int j = 1; j <= n; j++) {
		product *= j;
	}
	return product;
}

float binomialCoff(float n, float k) {
	float ans;
	ans = factorial(n) / (factorial(k) * factorial(n - k));
	return ans;
}

ColorVertex evaluateBezier(const float t, const std::vector<ColorVertex> *points) {
	ColorVertex result(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	float size = points->size();

	for (float i = 0; i < size; i++) {
		ColorVertex temp = points->at(i);

		result.position.x +=
				+ binomialCoff((size - 1), i)
				* pow(t, i) * pow((1 - t), (size - 1 - i))
				* temp.position.x;
		result.position.y +=
				+ binomialCoff((size - 1), i)
				* pow(t, i) * pow((1 - t), (size - 1 - i))
				* temp.position.y;
	}

	return result;
}

#endif
