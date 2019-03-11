//
// Wire
// Copyright (C) 2016 Wire Swiss GmbH
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <vector>
#include <map>
#ifdef __APPLE__
#include <AddressBook/ABAddressBookC.h>
#endif

typedef std::vector<std::string> stringvector;
typedef std::map<std::string, std::string> labelmap;
typedef std::vector<labelmap> labelvector;

class Person
{
  public:
	Person();
#ifdef __APPLE__
	Person(ABPersonRef p);
#endif

	const std::string &uniqueId() const
	{
		return m_uniqueId;
	}
	const std::string &firstName() const { return m_firstName; }
	const std::string &lastName() const { return m_lastName; }
	const std::string &nickname() const { return m_nickname; }
	const std::string &organization() const { return m_organization; }
	const std::string &title() const { return m_title; }
	const std::string &note() const { return m_note; }
	const std::string &street() const { return m_street; }
	const std::string &city() const { return m_city; }
	const std::string &state() const { return m_state; }
	const std::string &zip() const { return m_zip; }
	const std::string &country() const { return m_country; }
	const labelvector &phoneNumbers() const;
	const labelvector &emails() const;

  private:
#ifdef __APPLE__
	static std::string CFString2String(CFStringRef str);
	static std::string getStringProperty(ABPersonRef person, CFStringRef propertyName);
	static void fillLabelVector(ABPersonRef person, CFStringRef propertyName, labelvector &vec);
	static std::string getAddressProperty(ABPersonRef person, CFStringRef propertyName);
#endif
	std::string m_uniqueId;
	std::string m_firstName;
	std::string m_lastName;
	std::string m_nickname;
	std::string m_city;
	std::string m_street;
	std::string m_state;
	std::string m_zip;
	std::string m_country;
	std::string m_note;
	std::string m_organization;
	std::string m_title;
	labelvector m_phoneNumbers;
	labelvector m_emails;
};

#endif // PERSON_H
