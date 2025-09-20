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
    return swe_julday(date.year, date.month, date.day, date.hour, SE_GREG_CAL);
}

double calc_ut(
    double jd, 
    int *d, 
    int *h, 
    int *m, 
    int *index
) {
    double x2[6];
    char serr[256];
    swe_calc_ut(
        jd,
        SE_SUN,
        SEFLG_SPEED,
        x2,
        serr
    );
    double ra = x2[0] - swe_get_ayanamsa(jd);
    if (ra < 0) {
        ra += 360;
    }
    *index = ((int)ra / 30) % 12;
    double degree = ra - 30 * *index;
    *d = (int)degree;
    *h = (int)((degree - *d) * 60);
    *m = (int)((degree - *d - *h / 60.0) * 3600);
    return ra;
}

void find_zodiac_ages(date_t start, date_t end) {
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
        if (imonth == 3 && iday == 21) {
            int done = 0;
            for (double h = 0; h < 24.0; h += 1.0) {
                for (double m = 0; m < 60.0; m += 1.0) {
                    double new_jd = jd_start + h / 24.0 + m / (24.0 * 60.0);
                    int degree, hour, minute, index;
                    calc_ut(
                        new_jd, 
                        &degree, 
                        &hour, 
                        &minute,
                        &index
                    );
                    if (degree == 0 && hour == 0 && minute == 0) {
                        fprintf(
                            file,
                            "\"%d/03/21\",\"0\u00b0\",\"%s\"\n",
                            iyear,
                            SIGNS[index]
                        );
                        fflush(file);
                        done = 1;
                        jd_start += 360 * 1500;
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
    date_t start = {-13000, 1, 1, 0};
    date_t end = {14000, 1, 1, 0};
    swe_set_ephe_path("lib/swe/eph");
    swe_set_sid_mode(SE_SIDM_FAGAN_BRADLEY, 0, 0);
    find_zodiac_ages(start, end);
    return 0;
}
