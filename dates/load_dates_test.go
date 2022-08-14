package dates

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
	"yahrzeits-wall/config"
)

func TestLoadExampleDate(t *testing.T) {
	res, err := LoadHebcalDates(config.Date{
		Name:        "XXX",
		Gregorian:   "1985-01-20",
		AfterSunset: false,
	})
	assert.NoError(t, err)
	assert.NotEmpty(t, res)
	fmt.Print(res)
	res, err = LoadHebcalDates(config.Date{
		Name:        "XXX",
		Gregorian:   "1985-01-20",
		AfterSunset: true,
	})
	assert.NoError(t, err)
	assert.NotEmpty(t, res)
	fmt.Print(res)
}
