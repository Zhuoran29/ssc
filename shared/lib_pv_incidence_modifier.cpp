#include "lib_pv_incidence_modifier.h"


double transmittance(double theta1_deg, /* incidence angle of incoming radiation (deg) */
	double n_cover,  /* refractive index of cover material, n_glass = 1.586 */
	double n_incoming, /* refractive index of incoming material, typically n_air = 1.0 */
	double k,        /* proportionality constant assumed to be 4 (1/m) for derivation of Bouguer's law (set to zero to skip bougeur's law */
	double l_thick,  /* material thickness (set to zero to skip Bouguer's law */
	double *_theta2_deg) /* thickness of cover material (m), usually 2 mm for typical module */
{
	// reference: duffie & beckman, Ch 5.3

	double theta1 = theta1_deg * M_PI / 180.0;
	double theta2 = asin(n_incoming / n_cover * sin(theta1)); // snell's law, assuming n_air = 1.0
															  // fresnel's equation for non-reflected unpolarized radiation as an average of perpendicular and parallel components
	double tr = 1 - 0.5 *
		(pow(sin(theta2 - theta1), 2) / pow(sin(theta2 + theta1), 2)
			+ pow(tan(theta2 - theta1), 2) / pow(tan(theta2 + theta1), 2));

	if (_theta2_deg) *_theta2_deg = theta2 * 180 / M_PI;

	return tr * exp(-k * l_thick / cos(theta2));
}

double iam(double theta, bool ar_glass)
{
	if (theta < AOI_MIN) theta = AOI_MIN;
	if (theta > AOI_MAX) theta = AOI_MAX;

	double normal = iam_nonorm(1, ar_glass);
	double actual = iam_nonorm(theta, ar_glass);
	return actual / normal;
}

double iam_nonorm(double theta, bool ar_glass)
{
	double n_air = 1.0;

	double n_g = 1.526;
	double k_g = 4;
	double l_g = 0.002;

	double n_arc = 1.3;
	double k_arc = 4;
	double l_arc = l_g * 0.01;  // assume 1/100th thickness of glass for AR coating

	if (theta < AOI_MIN) theta = AOI_MIN;
	if (theta > AOI_MAX) theta = AOI_MAX;

	if (ar_glass)
	{
		double theta2 = 1;
		double tau_coating = transmittance(theta, n_arc, n_air, k_arc, l_arc, &theta2);
		double tau_glass = transmittance(theta2, n_g, n_arc, k_g, l_g);
		return tau_coating * tau_glass;
	}
	else
	{
		return transmittance(theta, n_g, n_air, k_g, l_g);
	}
}

double calculateIrradianceThroughCoverDeSoto(double theta, double theta_z, double tilt, double G_beam, double G_sky, double G_gnd)
{
	// establish limits on incidence angle and zenith angle
	if (theta < 1) theta = 1;
	if (theta > 89) theta = 89;

	if (theta_z > 86.0) theta_z = 86.0; // !Zenith angle must be < 90 (?? why 86?)
	if (theta_z < 0) theta_z = 0; 

	// transmittance at angle normal to surface (0 deg), use 1 (deg) to avoid numerical probs.
	double tau_norm = transmittance(1.0, n_cover, 1.0, k_trans, l_thick);

	// transmittance of beam radiation, at incidence angle
	double tau_beam = transmittance(theta, n_cover, 1.0, k_trans, l_thick);

	// transmittance of sky diffuse, at modified angle by (D&B Eqn 5.4.2)
	double theta_sky = 59.7 - 0.1388*tilt + 0.001497*tilt*tilt;
	double tau_sky = transmittance(theta_sky, n_cover, 1.0, k_trans, l_thick);

	// transmittance of ground diffuse, at modified angle by (D&B Eqn 5.4.1)
	double theta_gnd = 90.0 - 0.5788*tilt + 0.002693*tilt*tilt;
	double tau_gnd = transmittance(theta_gnd, n_cover, 1.0, k_trans, l_thick);

	// calculate component incidence angle modifiers, D&B Chap. 5 eqn 5.12.1, DeSoto'04
	double Kta_beam = tau_beam / tau_norm;
	double Kta_sky = tau_sky / tau_norm;
	double Kta_gnd = tau_gnd / tau_norm;

	// total effective irradiance absorbed by solar cell
	double Geff_total = G_beam * Kta_beam + G_sky * Kta_sky + G_gnd * Kta_gnd;

	if (Geff_total < 0) Geff_total = 0;

	return Geff_total;
}
