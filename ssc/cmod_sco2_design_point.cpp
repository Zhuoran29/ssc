#include "core.h"
#include "sco2_pc_core.h"

// This compute module finds the optimal cycle design that meets the user-input design point cycle efficiency
//    and calculates the required recuperator UA

static var_info _cm_vtab_sco2_design_point[] = {

	/*   VARTYPE   DATATYPE         NAME               LABEL                                                  UNITS     META  GROUP REQUIRED_IF CONSTRAINTS         UI_HINTS*/
	{ SSC_INPUT,  SSC_NUMBER,  "W_dot_net_des",   "Design cycle power output",                              "MW",         "",    "",      "*",     "",                "" },
	{ SSC_INPUT,  SSC_NUMBER,  "eta_c",           "Design compressor(s) isentropic efficiency",             "-",          "",    "",      "*",     "",                "" },
	{ SSC_INPUT,  SSC_NUMBER,  "eta_t",           "Design turbine isentropic efficiency",                   "-",          "",    "",      "*",     "",                "" },
	{ SSC_INPUT,  SSC_NUMBER,  "P_high_limit",    "High pressure limit in cycle",                           "MPa",        "",    "",      "*",     "",                "" },
	{ SSC_INPUT,  SSC_NUMBER,  "deltaT_PHX",      "Temp diff btw hot HTF and turbine inlet",                "C",          "",    "",      "*",     "",                "" },
																								           
	{ SSC_INPUT,  SSC_NUMBER,  "deltaT_ACC",      "Temp diff btw ambient air and compressor inlet",         "C",          "",    "",      "*",     "",                "" },
	{ SSC_INPUT,  SSC_NUMBER,  "T_amb_des",       "Design: Ambient temperature for air cooler",             "C",          "",    "",      "*",     "",                "" },
																								           
	{ SSC_INPUT,  SSC_NUMBER,  "T_htf_hot_des",   "Tower design outlet temp",                               "C",          "",    "",      "*",     "",                "" },
	{ SSC_INPUT,  SSC_NUMBER,  "eta_des",         "Power cycle thermal efficiency",                         "",           "",    "",      "*",     "",                "" },
																								           
	{ SSC_INPUT,  SSC_NUMBER,  "run_off_des_study", "1 = yes, 0/other = no",                                "",           "",    "",      "*",      "",               "" },
	{ SSC_INPUT,  SSC_ARRAY,   "part_load_fracs", "Array of part load q_dot_in fractions for off-design parametric", "",  "",    "",      "run_off_des_study=1", "",  "" },
	{ SSC_INPUT,  SSC_ARRAY,   "T_amb_array",     "Array of ambient temperatures for off-design parametric","C",          "",    "",      "run_off_des_study=1", "",  "" },

	{ SSC_OUTPUT, SSC_NUMBER,  "eta_thermal_calc","Calculated cycle thermal efficiency",                    "-",          "",    "",      "*",     "",                "" },
	{ SSC_OUTPUT, SSC_NUMBER,  "UA_total",        "Total recuperator UA",                                   "kW/K",       "",    "",      "*",     "",                "" },
	{ SSC_OUTPUT, SSC_NUMBER,  "recomp_frac",     "Recompression fraction",                                 "-",          "",    "",      "*",     "",                "" },
	{ SSC_OUTPUT, SSC_NUMBER,  "P_comp_in",       "Compressor inlet pressure",                              "MPa",        "",    "",      "*",     "",                "" },
	{ SSC_OUTPUT, SSC_NUMBER,  "P_comp_out",      "Compressor outlet pressure",                             "MPa",        "",    "",      "*",     "",                "" },
	{ SSC_OUTPUT, SSC_NUMBER,  "T_htf_cold",      "Calculated cold HTF temp",                               "C",          "",    "",      "*",     "",                "" },

	{ SSC_OUTPUT, SSC_ARRAY,   "part_load_fracs_out", "Array of part load fractions that SOLVED at off design", "-",      "",    "",      "run_off_des_study=1", "",  "" },
	{ SSC_OUTPUT, SSC_ARRAY,   "part_load_eta",   "Matrix of power cycle efficiency results for q_dot_in part load", "-", "",    "",      "run_off_des_study=1", "",  "" },
	{ SSC_OUTPUT, SSC_ARRAY,   "part_load_coefs", "Part load polynomial coefficients",                      "-",          "",    "",      "run_off_des_study=1", "",  "" },
	{ SSC_OUTPUT, SSC_NUMBER,  "part_load_r_squared", "Part load curve fit R squared",                      "-",          "",    "",      "run_off_des_study=1", "",  "" },
	{ SSC_OUTPUT, SSC_ARRAY,   "T_amb_array_out", "Array of ambient temps that SOLVED at off design",       "C",          "",    "",      "run_off_des_study=1", "",  "" },
	{ SSC_OUTPUT, SSC_ARRAY,   "T_amb_eta",       "Matrix of ambient temps and power cycle efficiency",     "-",          "",    "",      "run_off_des_study=1", "",  "" },
	{ SSC_OUTPUT, SSC_ARRAY,   "T_amb_coefs",     "Part load polynomial coefficients",                      "-",          "",    "",      "run_off_des_study=1", "",  "" },
	{ SSC_OUTPUT, SSC_NUMBER,  "T_amb_r_squared", "T amb curve fit R squared",                              "-",          "",    "",      "run_off_des_study=1", "",  "" },

	var_info_invalid };

class cm_sco2_design_point : public compute_module
{
public:

	cm_sco2_design_point()
	{
		add_var_info(_cm_vtab_sco2_design_point);
	}

	void exec() throw(general_error)
	{
		// Get user-defined parameters
		double W_dot_net_des = as_double("W_dot_net_des")*1.E3;
		double eta_c = as_double("eta_c");
		double eta_t = as_double("eta_t");
		double P_high_limit = as_double("P_high_limit")*1.E3;		//[kPa], convert from MPa
		double delta_T_t = as_double("deltaT_PHX");

		double delta_T_acc = as_double("deltaT_ACC");
		double T_amb_cycle_des = as_double("T_amb_des")+273.15;

		double T_htf_hot = as_double("T_htf_hot_des")+273.15;
		double eta_thermal_des = as_double("eta_des");

		// Define hardcoded sco2 design point parameters
		std::vector<double> DP_LT(2);
 		/*(cold, hot) positive values are absolute [kPa], negative values are relative (-)*/
                DP_LT[0] = 0;
                DP_LT[1] = 0;
		/*(cold, hot) positive values are absolute [kPa], negative values are relative (-)*/
		std::vector<double> DP_HT(2);
                DP_HT[0] = 0;  
                DP_HT[1] = 0;  
		/*(cold, hot) positive values are absolute [kPa], negative values are relative (-)*/
		std::vector<double> DP_PC(2);  
                DP_PC[0] = 0;  
                DP_PC[1] = 0;  
		/*(cold, hot) positive values are absolute [kPa], negative values are relative (-)*/
		std::vector<double> DP_PHX(2);
                DP_PHX[0] = 0;  
                DP_PHX[1] = 0;  
		int N_sub_hxrs = 10;
		double N_t_des = 3600.0;
		double tol = 1.E-3;
		double opt_tol = 1.E-3;

		// Initialize cycle here, so can use 'get_design_limits()'
			// Also define error and warning message strings
		std::string error_msg;
		error_msg[0] = NULL;
		int error_code = 0;
		C_RecompCycle rc_cycle;

		int run_off_des_study = as_integer("run_off_des_study");

		if( run_off_des_study == 1 && eta_thermal_des < 0.0 )
		{
			// Find optimal design at maximum allowable recuperator UA
			// This will result in the maximum allowable cycle efficiency
			// Subtract abs(eta_thermal_des) from max allowable efficiency to get "reasonable" efficiency to use in subsequent analysis

			C_RecompCycle::S_auto_opt_design_parameters rc_params_max_eta;

			rc_params_max_eta.m_W_dot_net = W_dot_net_des;					//[kW]
			rc_params_max_eta.m_T_mc_in = T_amb_cycle_des + delta_T_acc;	//[K]
			rc_params_max_eta.m_T_t_in = T_htf_hot - delta_T_t;				//[K]
			rc_params_max_eta.m_DP_LT = DP_LT;
			rc_params_max_eta.m_DP_HT = DP_HT;
			rc_params_max_eta.m_DP_PC = DP_PC;
			rc_params_max_eta.m_DP_PHX = DP_PHX;

			// Get max recuperator UA!!!
			rc_params_max_eta.m_UA_rec_total = rc_cycle.get_design_limits().m_UA_net_power_ratio_max*rc_params_max_eta.m_W_dot_net;		//[kW/K]

			rc_params_max_eta.m_eta_mc = eta_c;
			rc_params_max_eta.m_eta_rc = eta_c;
			rc_params_max_eta.m_eta_t = eta_t;
			rc_params_max_eta.m_N_sub_hxrs = N_sub_hxrs;
			rc_params_max_eta.m_P_high_limit = P_high_limit;
			rc_params_max_eta.m_tol = tol;
			rc_params_max_eta.m_opt_tol = opt_tol;
			rc_params_max_eta.m_N_turbine = N_t_des;

			rc_cycle.auto_opt_design(rc_params_max_eta, error_code);

			if(error_code != 0)
			{
				throw exec_error("sCO2 maximum efficiency calculations failed","");
			}

			// Get solved maximum cycle efficiency and subtract delta_eta to get "reasonable" efficiency
			eta_thermal_des = rc_cycle.get_design_solved()->m_eta_thermal - fabs(eta_thermal_des);		//[-]
		}		

		C_RecompCycle::S_auto_opt_design_hit_eta_parameters rc_params;
		rc_params.m_W_dot_net = W_dot_net_des;					//[kW]
		rc_params.m_eta_thermal = eta_thermal_des;
		rc_params.m_T_mc_in = T_amb_cycle_des + delta_T_acc;	//[K]
		rc_params.m_T_t_in = T_htf_hot - delta_T_t;				//[K]
		rc_params.m_DP_LT = DP_LT;
		rc_params.m_DP_HT = DP_HT;
		rc_params.m_DP_PC = DP_PC;
		rc_params.m_DP_PHX = DP_PHX;
		rc_params.m_eta_mc = eta_c;
		rc_params.m_eta_rc = eta_c;
		rc_params.m_eta_t = eta_t;
		rc_params.m_N_sub_hxrs = N_sub_hxrs;
		rc_params.m_P_high_limit = P_high_limit;
		rc_params.m_tol = tol;
		rc_params.m_opt_tol = opt_tol;
		rc_params.m_N_turbine = N_t_des;			

		rc_cycle.auto_opt_design_hit_eta(rc_params, error_code, error_msg);

		if(error_code != 0)
		{
			throw exec_error("sco2 design point calcs", error_msg);
		}

		double eta_thermal_calc = rc_cycle.get_design_solved()->m_eta_thermal;
		double UA_total = rc_cycle.get_design_solved()->m_UA_HT + rc_cycle.get_design_solved()->m_UA_LT;
		double recomp_frac = rc_cycle.get_design_solved()->m_recomp_frac;
		double P_comp_in = rc_cycle.get_design_solved()->m_pres[0] / 1.E3;
		double P_comp_out = rc_cycle.get_design_solved()->m_pres[1] / 1.E3;
		double T_htf_cold = rc_cycle.get_design_solved()->m_temp[5 - 1] + delta_T_t - 273.15;	//[C]

		// Assign SSC outputs
		assign("eta_thermal_calc", eta_thermal_calc);
		assign("UA_total", UA_total);
		assign("recomp_frac", recomp_frac);
		assign("P_comp_in", P_comp_in);
		assign("P_comp_out", P_comp_out);
		assign("T_htf_cold", T_htf_cold); 

		if( error_msg[0] == NULL )
			log("Design point optimization was successful!");
		else
		{
			log("The sCO2 design point optimization solved with the following warning(s):\n" + error_msg);
		}

		//int run_off_des_study = as_integer("run_off_des_study");

		if( run_off_des_study != 1)
		{
			return;
		}
		else	// Run off-design parametrics
		{
			C_RecompCycle::S_opt_target_od_parameters opt_target_od_params;
			
			// opt_target_od_params.m_T_mc_in      ** Set compressor inlet temperature below **
			opt_target_od_params.m_T_t_in = rc_params.m_T_t_in;			//[K]
			
			// opt_target_od_params.m_target       ** Set target q_dot below **
			opt_target_od_params.m_is_target_Q = true;

			opt_target_od_params.m_N_sub_hxrs = rc_params.m_N_sub_hxrs;
			opt_target_od_params.m_lowest_pressure = 1000.0;
			opt_target_od_params.m_highest_pressure = 17000.0;

			opt_target_od_params.m_recomp_frac_guess = rc_cycle.get_design_solved()->m_recomp_frac;
			opt_target_od_params.m_fixed_recomp_frac = false;

			opt_target_od_params.m_N_mc_guess = rc_cycle.get_design_solved()->m_N_mc;
			opt_target_od_params.m_fixed_N_mc = false;

			opt_target_od_params.m_N_t_guess = rc_cycle.get_design_solved()->m_N_t;
			opt_target_od_params.m_fixed_N_t = true;

			opt_target_od_params.m_tol = rc_params.m_tol;
			opt_target_od_params.m_opt_tol = rc_params.m_opt_tol;

			opt_target_od_params.m_use_default_res = false;

			double q_dot_in_des = rc_cycle.get_design_solved()->m_W_dot_net / rc_cycle.get_design_solved()->m_eta_thermal;	//[kWt]

			size_t n_f_pl = -1;
			ssc_number_t * f_pl = as_array("part_load_fracs", &n_f_pl);

			int n_solved = 0;

			std::vector<double> part_load_fracs_out(0);
			std::vector<double> part_load_eta(0);

			if(n_f_pl > 0)	// At least one part-load simulation is required
			{
				opt_target_od_params.m_T_mc_in = rc_params.m_T_mc_in;		//[K]

				for( int i = 0; i < n_f_pl; i++ )
				{
					opt_target_od_params.m_target = (double)f_pl[i] * q_dot_in_des;				//[kWt]
					log(util::format("Off design simulation at part load = %lg", f_pl[i]));
					int od_error_code = 0;
					rc_cycle.optimal_target_off_design(opt_target_od_params, od_error_code);
					if(od_error_code == 0)
					{
						part_load_fracs_out.push_back((double)f_pl[i]);
						part_load_eta.push_back(rc_cycle.get_od_solved()->m_eta_thermal/eta_thermal_calc);
					}
				}

				n_solved = part_load_fracs_out.size();
			}
						
			ssc_number_t * f_pl_out = allocate("part_load_fracs_out", n_solved);
			ssc_number_t * eta_f_pl = allocate("part_load_eta", n_solved);

			for( int i = 0; i < n_solved; i++ )
			{
				f_pl_out[i] = part_load_fracs_out[i];
				eta_f_pl[i] = part_load_eta[i];
			}

			// Find and write polynomial coefficients for part load
			std::vector<double> pl_coefs;
			double pl_r_squared = std::numeric_limits<double>::quiet_NaN();
			bool pl_success = find_polynomial_coefs(part_load_fracs_out, part_load_eta, 5, pl_coefs, pl_r_squared);
			assign("Part_load_r_squared", pl_r_squared);

			ssc_number_t * p_pl_coefs = allocate("part_load_coefs", 5);
			if(pl_success)
			{
				for( int i = 0; i < 5; i++ )
					p_pl_coefs[i] = pl_coefs[i];
			}
			else
			{
				log("Part load coefficient calcuations failed");
				for( int i = 0; i < 5; i++ )
					p_pl_coefs[i] = 0.0;
			}
			// ********************************************

						
			size_t n_T_amb_od = -1;
			ssc_number_t * T_amb_od = as_array("T_amb_array", &n_T_amb_od);

			n_solved = 0;

			std::vector<double> T_amb_out(0);
			std::vector<double> T_amb_eta(0);

			if(n_T_amb_od > 0)	// At least one off-design ambient temperature simulation is required
			{
				opt_target_od_params.m_target = q_dot_in_des;

				for( int i = 0; i < n_T_amb_od; i++ )
				{
					opt_target_od_params.m_T_mc_in = max(rc_cycle.get_design_limits().m_T_mc_in_min, (double)T_amb_od[i] + delta_T_acc + 273.15);	//[K] convert from C and add air cooler deltaT
					log(util::format("Off design simulation at ambient temperature = %lg", T_amb_od[i]));
					log(util::format("Corresponding compressor inlet temperature = %lg", opt_target_od_params.m_T_mc_in - 273.15));
					int od_error_code = 0;
					rc_cycle.optimal_target_off_design(opt_target_od_params, od_error_code);
					if(od_error_code == 0)
					{
						T_amb_out.push_back((double)T_amb_od[i]);
						T_amb_eta.push_back(rc_cycle.get_od_solved()->m_eta_thermal / eta_thermal_calc);
					}
				}

				n_solved = T_amb_out.size();
			}						

			ssc_number_t * T_amb_od_out = allocate("T_amb_array_out", n_solved);
			ssc_number_t * eta_T_amb = allocate("T_amb_eta", n_solved);

			for( int i = 0; i < n_solved; i++ )
			{
				T_amb_od_out[i] = T_amb_out[i];
				eta_T_amb[i] = T_amb_eta[i];
			}


			// Find polynomial coefficients for part load
			std::vector<double> T_amb_coefs;
			std::vector<double> T_amb_od_less_des;
			double T_amb_r_squared = std::numeric_limits<double>::quiet_NaN();

			T_amb_od_less_des.resize(n_solved);
			for( int i = 0; i < n_solved; i++ )
			{
				T_amb_od_less_des[i] = T_amb_od[i] - (T_amb_cycle_des - 273.15);
			}
			bool T_amb_success = find_polynomial_coefs(T_amb_od_less_des, T_amb_eta, 5, T_amb_coefs, T_amb_r_squared);
			assign("T_amb_r_squared", T_amb_r_squared);

			ssc_number_t * p_T_amb_coefs = allocate("T_amb_coefs", 5);
			if( T_amb_success )
			{
				for( int i = 0; i < 5; i++ )
					p_T_amb_coefs[i] = T_amb_coefs[i];
			}
			else
			{
				log("Ambient temperature coefficient calcuations failed");
				for( int i = 0; i < 5; i++ )
					p_T_amb_coefs[i] = 0.0;
			}

			// ******************************************************
		}

	}


};

DEFINE_MODULE_ENTRY(sco2_design_point, "Returns optimized sco2 cycle parameters given inputs", 0)
