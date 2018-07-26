#include <gtest/gtest.h>
#include <lib_shared_inverter.h>

/**
* Shared Inverter Class test
*/

class sharedInverterTest : public ::testing::Test {
protected:
	SharedInverter* inv;
	sandia_inverter_t sinv;
	partload_inverter_t plinv;
	double pAC = 100.;
	double eff = 0.;
	double loss = 0.;
	double e = 0.01;
public:
	void reset() {
		pAC = 100;
		eff = 1.;
		loss = 0.;
	}
	void SetUp() {
		inv = new SharedInverter(0, 1, &sinv, &plinv);
	}
	void TearDown() {
		if (inv) delete inv;
	}
};


TEST_F(sharedInverterTest, tempDerateTest_lib_shared_inverter) {
	/// First set of curves
	std::vector<double> c1 = { 200., 20., -0.2, 40., -0.4 };
	std::vector<double> c2 = { 300., 30., -0.3, 60., -0.6 };
	std::vector<std::vector<double>> curves = { c1, c2 };
	EXPECT_FALSE(inv->setTempDerateCurves(curves)) << "set up temp derate set 1";

	// zero efficiency error case
	double V = 200.;
	double T = 5.;
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 100., e) << "zero efficiency error case";

	// zero power error case
	pAC = 0.;
	eff = 1.;
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 0, e) << "zero power error case";

	// no derate cases
	pAC = 100.;
	eff = 1.;
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 100, e) << "no derate";

	// V less than that of first curve, extrapolated startT is 10. and slope is -0.1
	V = 100.;
	T = 11.;
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 90, e) << "case 1";

	// V more than that of first curve, interpolated startT is 25. and slope is -0.25
	V = 250.;
	T = 26.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 75, e) << "case 2";

	// V more than that of second curve, interpolated startT is 40. and slope is -0.4
	V = 400.;
	T = 41.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 60, e) << "case 3";

	/// Second set of curves, different # of pairs
	std::vector<double> c3 = { 200., 20., -0.2};
	std::vector<double> c4 = { 300., 30., -0.3, 60., -0.6 };
	std::vector<std::vector<double>> curves2 = { c3, c4 };
	EXPECT_FALSE(inv->setTempDerateCurves(curves2)) << "set up temp derate set 2";

	// V<200 and T < 10: extrapolated startT is 10. and slope is -0.1
	V = 100.;
	T = 9.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 100, e) << "case 7";

	// V<200 and 10 < T: extrapolated startT is 10. and slope is -0.1
	V = 100.;
	T = 11.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 90, e) << "case 7";

	// 200<V<300 and T < 25, interpolated startT is 25. and slope is -0.25
	V = 250.;
	T = 24.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 100, e) << "case 9";

	// 200<V<300 and 25 < T < 40, interpolated startT is 25. and slope is -0.25
	V = 250.;
	T = 26.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 75, e) << "case 9";

	// 200<V<300 and 40 < T, interpolated startT is 40. and slope is -0.4
	V = 250.;
	T = 41.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 60, e) << "case 9";

	// 300<V and T < 40, extrapolated startT is 40. and slope is -0.4
	V = 400.;
	T = 9.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 100, e) << "case 9";

	// 300<V and 40 < T < 100, interpolated startT is 40. and slope is -0.4
	V = 400.;
	T = 41.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 60, e) << "case 9";

	/// Third set of curves, different # of pairs
	std::vector<double> c5 = { 200., 20., -0.2, 60, -0.6 };
	std::vector<double> c6 = { 300., 30., -0.3};
	std::vector<std::vector<double>> curves3 = { c5, c6 };
	EXPECT_FALSE(inv->setTempDerateCurves(curves3)) << "set up temp derate set 3";

	// V<200 and T < 10: extrapolated startT is 10. and slope is -0.1
	V = 100.;
	T = 9.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 100, e) << "case 7";

	// V<200 and 10 < T : extrapolated startT is 10. and slope is -0.1
	V = 100.;
	T = 11.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 90, e) << "case 7";

	// 200<V<300 and T < 25, interpolated startT is 25. and slope is -0.25
	V = 250.;
	T = 24.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 100, e) << "case 9";

	// 200<V<300 and 25 < T < 40, interpolated startT is 25. and slope is -0.25
	V = 250.;
	T = 26.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 75, e) << "case 9";

	// 200<V<300 and 40 < T, interpolated startT is 45. and slope is -0.45
	V = 250.;
	T = 46.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 55, e) << "case 9";

	// 300<V and T < 40, extrapolated startT is 40. and slope is -0.4
	V = 400.;
	T = 9.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 100, e) << "case 9";

	// 300<V and 40 < T, interpolated startT is 40. and slope is -0.4
	V = 400.;
	T = 41.;
	reset();
	inv->calculateTempDerate(V, T, pAC, eff, loss);
	EXPECT_NEAR(pAC, 60, e) << "case 9";

}
