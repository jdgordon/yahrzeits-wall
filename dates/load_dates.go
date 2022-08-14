package dates

import (
	"fmt"
	"io"
	"net/http"
	"strings"
	"time"

	"gopkg.in/yaml.v2"
	"yahrzeits-wall/config"
)

const (
	numYears = 2
	hebcalYahrzeitUrl = "https://www.hebcal.com/yahrzeit"
)

type hebcalRes struct {
	Title string
	Date time.Time
	Range interface{}
	Items []struct{
		Date string
		Hdate string
	}
}

// NightOf is a reminder that the date included starts the yahrzeit on that evening.
type NightOf time.Time
func (n NightOf) String() string { return time.Time(n).Format("2006-01-02 evening") }

// LoadHebcalDates loads the list of aniversary dates for the given date.
// the date hebcal returns is the full day of Yahrzeit, so we need to return the previous day
func LoadHebcalDates(d config.Date) ([]NightOf, error) {
	client := http.DefaultClient

	date, err := time.Parse("2006-01-02", d.Gregorian)
	if err != nil {
		return nil, err
	}
	req := fmt.Sprintf("cfg=json&v=yahrzeit&years=%d&y1=%4d&m1=%d&d1=%d",
		numYears, date.Year(), date.Month(), date.Day())
	if d.AfterSunset {
		req += "&s1=on"
	}
	resp, err := client.Post(hebcalYahrzeitUrl, "application/x-www-form-urlencoded", strings.NewReader(req))
	if err != nil {
		return nil, err
	}
	if resp.Body == nil {
		return nil, io.EOF
	}
	defer resp.Body.Close()
	content, _ := io.ReadAll(resp.Body)

	var res hebcalRes
	if err := yaml.Unmarshal(content, &res); err != nil {
		return nil, err
	}
	var out []NightOf
	for _, item := range res.Items {
		hdate, err := time.Parse("2006-01-02", item.Date)
		if err != nil {
			return nil, err
		}
		out = append(out, NightOf(hdate.AddDate(0, 0, -1)))
	}
	return out, nil
}