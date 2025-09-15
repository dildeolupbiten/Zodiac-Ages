#include "./lib/swe/swephexp.h"

static const char *SIGNS[12] = {
    "Ari", "Tau", "Gem", "Can", "Leo", "Vir",
    "Lib", "Sco", "Sag", "Cap", "Aqu", "Pis"
};

typedef struct date_t {
    int year;
    int month;
    int day;
    double hour;
} date_t;

double julday(date_t date) {
    double tjd_ut = swe_julday(
        date.year, 
        date.month, 
        date.day, 
        date.hour, 
        SE_GREG_CAL
    );
    return tjd_ut + swe_deltat(tjd_ut);
}

double calc_ut(
    int body, 
    double jd, 
    int *d, 
    int *h, 
    int *m, 
    int *sign_index,
    int zodiac
) {
    double diff = zodiac != -1 ? swe_get_ayanamsa(jd) : 0;
    double x2[6];
    char serr[256];
    swe_calc_ut(
        jd,
        body,
        SEFLG_SPEED,
        x2,
        serr
    );
    double raw_degree = x2[0] - diff;
    if (raw_degree < 0) {
        raw_degree += 360;
    }
    *sign_index = ((int)raw_degree / 30) % 12;
    double sign_degree = raw_degree - 30 * *sign_index;
    *d = (int)sign_degree;
    *h = (int)((sign_degree - *d) * 60);
    *m = (int)((sign_degree - *d - *h / 60.0) * 3600);
    return raw_degree;
}

void find_dates_for_body_positions(
    int body,
    date_t start, 
    date_t end,
    int month,
    int day,
    int degree,
    int hour,
    int minute,
    int zodiac,
    int jd_step
) {
    FILE *file = fopen("output.csv", "w");
    if (!file) {
        perror("Can't open file");
        return;
    }
    fprintf(file, "Date,Ecliptic Longitude,Sign\n");
    double jd_start = julday(start);
    double jd_end = julday(end);
    while (jd_start < jd_end) {
        int iyear, imonth, iday, ihour, imin;
        double isec;
        swe_jdut1_to_utc(
            jd_start, 
            SE_GREG_CAL, 
            &iyear, 
            &imonth, 
            &iday, 
            &ihour, 
            &imin, 
            &isec
        );
        if (imonth == month && iday == day) {
            int done = 0;
            for (double h = 0; h < 24.0; h += 1.0) {
                for (double m = 0; m < 60.0; m += 1.0) {
                    double new_jd = jd_start + h / 24.0 + m / (24.0 * 60.0);
                    int body_degree, body_hour, body_minute, sign_index;
                    calc_ut(
                        body, 
                        new_jd, 
                        &body_degree, 
                        &body_hour, 
                        &body_minute,
                        &sign_index,
                        zodiac
                    );
                    if (
                        body_degree == degree && 
                        body_hour == hour && 
                        body_minute == minute
                    )
                    {
                        fprintf(
                            file,
                            "%d/%s%d/%s%d,%s%d\u00b0%s%d'%s%d\",%s\n",
                            iyear,
                            (imonth > 9 ? "" : "0"), imonth,
                            (iday > 9 ? "" : "0"), iday,
                            (body_degree > 9 ? "" : "0"), body_degree,
                            (body_minute > 9 ? "" : "0"), body_minute,
                            (body_hour > 9 ? "" : "0"), body_hour,
                            SIGNS[sign_index]
                        );
                        fflush(file);
                        done = 1;
                        jd_start += jd_step;
                        break;
                    }
                }
                if (done) {
                    break;
                }
            }
        }
        jd_start++;
    }
    fclose(file);
}

int main() {
    int month = 3;
    int day = 21;
    int degree = 0;
    int hour = 0;
    int minute = 0;
    int zodiac = 1;
    int body = 0;
    int jd_step = 360 * 1500;
    date_t start = {-13000, 1, 1, 0};
    date_t end = {14000, 1, 1, 0};
    swe_set_ephe_path("lib/swe/eph");
    swe_set_sid_mode(1, 0, 0);
    find_dates_for_body_positions(
        body, start, end, month, day, degree, hour, minute, zodiac, jd_step
    );
    return 0;
}
