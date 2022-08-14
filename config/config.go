package config

type Date struct {
	Name string
	Gregorian string
	AfterSunset bool
}

type DateFile struct {
	Dates map[int]Date
}

type LightCommand struct {

	Items []int
}

type CommandFile map[string]LightCommand