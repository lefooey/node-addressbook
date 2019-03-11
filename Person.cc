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

#include "Person.h"
#include "AddressBook.h"

#ifdef __APPLE__
std::string Person::CFString2String(CFStringRef str)
{
	std::string rv;
	CFIndex length = CFStringGetLength(str);
	CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
	char *buffer = (char *)malloc(maxSize);
	if (CFStringGetCString(str, buffer, maxSize, kCFStringEncodingUTF8))
	{
		rv = buffer;
		free(buffer);
	}

	return rv;
}

std::string Person::getStringProperty(ABPersonRef person, CFStringRef propertyName)
{
	CFStringRef propertyVal = (CFStringRef)ABRecordCopyValue(person, propertyName);
	std::string rv;

	if (propertyVal && CFGetTypeID(propertyVal) == CFStringGetTypeID())
	{
		rv = CFString2String(propertyVal);
		CFRelease(propertyVal);
	}

	return rv;
}

void Person::fillLabelVector(ABPersonRef person, CFStringRef propertyName, labelvector &vec)
{
	ABMultiValueRef propertyArray = (ABMultiValueRef)ABRecordCopyValue(person, propertyName);

	if (propertyArray)
	{
		CFIndex count = ABMultiValueCount(propertyArray);
		for (CFIndex p = 0; p < count; p++)
		{
			labelmap vm;
			CFStringRef propertyLabel = (CFStringRef)ABMultiValueCopyLabelAtIndex(propertyArray, p);
			CFStringRef propertyVal = (CFStringRef)ABMultiValueCopyValueAtIndex(propertyArray, p);
			vm.insert(std::pair<std::string, std::string>("type", CFString2String(propertyLabel)));
			vm.insert(std::pair<std::string, std::string>("value", CFString2String(propertyVal)));
			vec.push_back(vm);
			CFRelease(propertyLabel);
			CFRelease(propertyVal);
		}
		CFRelease(propertyArray);
	}
}

std::string Person::getAddressProperty(ABPersonRef person, CFStringRef propertyName)
{
	ABMultiValueRef addressList = (ABMultiValueRef)ABRecordCopyValue(person, kABAddressProperty);
	std::string rv;

	if (addressList)
	{
		CFStringRef primaryIdentifier = ABMultiValueCopyPrimaryIdentifier(addressList);
		int primaryIndex = ABMultiValueIndexForIdentifier(addressList, primaryIdentifier);
		CFRelease(primaryIdentifier);
		CFDictionaryRef addr = (CFDictionaryRef)ABMultiValueCopyValueAtIndex(addressList, primaryIndex);
		if (addr)
		{
			CFStringRef propertyVal = (CFStringRef)CFDictionaryGetValue(addr, propertyName);
			rv = CFString2String(propertyVal);
			CFRelease(addr);
		}
		CFRelease(addressList);
	}
	return rv;
}

#endif

Person::Person()
{
}

#ifdef __APPLE__
Person::Person(ABPersonRef p)
{
	m_uniqueId = getStringProperty(p, kABUIDProperty);
	m_firstName = getStringProperty(p, kABFirstNameProperty);
	m_lastName = getStringProperty(p, kABLastNameProperty);
	m_nickname = getStringProperty(p, kABNicknameProperty);
	m_organization = getStringProperty(p, kABOrganizationProperty);
	m_title = getStringProperty(p, kABTitleProperty);
	m_note = getStringProperty(p, kABNoteProperty);
	m_street = getAddressProperty(p, kABAddressStreetKey);
	m_city = getAddressProperty(p, kABAddressCityKey);
	m_state = getAddressProperty(p, kABAddressStateKey);
	m_zip = getAddressProperty(p, kABAddressZIPKey);
	m_country = getAddressProperty(p, kABAddressCountryKey);

	fillLabelVector(p, kABEmailProperty, m_emails);
	fillLabelVector(p, kABPhoneProperty, m_phoneNumbers);
}
#endif

const labelvector &Person::phoneNumbers() const
{
	return m_phoneNumbers;
}

const labelvector &Person::emails() const
{
	return m_emails;
}
