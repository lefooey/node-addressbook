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

std::string Person::CFData2String(CFDataRef data)
{
	std::string rv;
	CFIndex length = CFDataGetLength(data);
	if (length) {
		unsigned char *buffer = (unsigned char *)malloc(length+1);
		CFDataGetBytes(data, CFRangeMake(0,length), buffer);
		rv = base64_encode(buffer, length);
		free(buffer);
	} else {
		rv = "";
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
			if (propertyVal) {
				rv = CFString2String(propertyVal);
				CFRelease(propertyVal);
			} else {
				rv = "";
			}
		}
	}
	return rv;
}

std::string Person::getImageProperty(ABPersonRef person)
{
	std::string rv;
	CFDataRef imageData = ABPersonCopyImageData(person);
	if (imageData) {
		rv = CFData2String(imageData);
		CFRelease(imageData);
	} else {
		rv = "";
	}
	return rv;
}

#endif

std::string Person::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += person_base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; (j < i + 1); j++)
      ret += person_base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}


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
	m_image = getImageProperty(p);
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
