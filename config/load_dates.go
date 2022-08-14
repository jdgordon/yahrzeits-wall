package config

import (
	"gopkg.in/yaml.v2"
)

func LoadDates(content []byte) (DateFile, error) {
	var result DateFile
	if err := yaml.UnmarshalStrict(content, &result); err != nil{
		return DateFile{}, err
	}
	return result, nil
}
