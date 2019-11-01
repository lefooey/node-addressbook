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

#include <nan.h>
#include <functional>
#include <iostream>
#include "AddressBook.h"

using namespace Nan;
using namespace std;
using namespace v8;

void setVectorArray(Isolate *isolate, Local<Object> obj, const char *name, const labelvector &src)
{
	Local<Array> array = Array::New(isolate);
	for (unsigned int i = 0; i < src.size(); i++)
	{
		labelmap s = src[i];
		Local<Object> item = Object::New(isolate);
		Nan::Set(item, New<String>("type").ToLocalChecked(),
				  String::NewFromUtf8(isolate, s["type"].c_str(), NewStringType::kNormal).ToLocalChecked());
		Nan::Set(item, New<String>("value").ToLocalChecked(),
				String::NewFromUtf8(isolate, s["value"].c_str(), NewStringType::kNormal).ToLocalChecked());
		Nan::Set(array, i, item);
	}
	Nan::Set(obj, String::NewFromUtf8(isolate, name, NewStringType::kNormal).ToLocalChecked(), array);
}

void setNameObject(Isolate *isolate, Local<Object> obj, Person *person)
{
	Local<Object> name = Object::New(isolate);
	Nan::Set(name, New<String>("givenName").ToLocalChecked(),
			  String::NewFromUtf8(isolate, person->firstName().c_str(), NewStringType::kNormal).ToLocalChecked());
	Nan::Set(name, New<String>("familyName").ToLocalChecked(),
			  String::NewFromUtf8(isolate, person->lastName().c_str(), NewStringType::kNormal).ToLocalChecked());
	Nan::Set(obj, New<String>("name").ToLocalChecked(), name);
}

void setOrganizationObject(Isolate *isolate, Local<Object> obj, Person *person)
{
	Local<Object> organization = Object::New(isolate);
	Nan::Set(organization, New<String>("name").ToLocalChecked(),
					  String::NewFromUtf8(isolate, person->organization().c_str(), NewStringType::kNormal).ToLocalChecked());
	Nan::Set(organization, New<String>("title").ToLocalChecked(),
					  String::NewFromUtf8(isolate, person->title().c_str(), NewStringType::kNormal).ToLocalChecked());

	Local<Array> organizationList = Array::New(isolate);
	Nan::Set(organizationList, 0, organization);

	Nan::Set(obj, New<String>("organizations").ToLocalChecked(), organizationList);
}

void setAddressObject(Isolate *isolate, Local<Object> obj, Person *person)
{
	Local<Object> address = Object::New(isolate);
	Nan::Set(address, New<String>("pref").ToLocalChecked(), True());
	Nan::Set(address, New<String>("streetAddress").ToLocalChecked(),
				 String::NewFromUtf8(isolate, person->street().c_str(), NewStringType::kNormal).ToLocalChecked());
	Nan::Set(address, New<String>("locality").ToLocalChecked(),
				 String::NewFromUtf8(isolate, person->city().c_str(), NewStringType::kNormal).ToLocalChecked());
	Nan::Set(address, New<String>("region").ToLocalChecked(),
				 String::NewFromUtf8(isolate, person->state().c_str(), NewStringType::kNormal).ToLocalChecked());
	Nan::Set(address, New<String>("postalCode").ToLocalChecked(),
				 String::NewFromUtf8(isolate, person->zip().c_str(), NewStringType::kNormal).ToLocalChecked());
	Nan::Set(address, New<String>("country").ToLocalChecked(),
				 String::NewFromUtf8(isolate, person->country().c_str(), NewStringType::kNormal).ToLocalChecked());

	Local<Array> addressList = Array::New(isolate);
	Nan::Set(addressList, 0, address);

	Nan::Set(obj, New<String>("addresses").ToLocalChecked(), addressList);
}

void fillPersonObject(Isolate *isolate, Local<Object> obj, Person *person)
{
	Nan::Set(obj, New<String>("id").ToLocalChecked(),
			 String::NewFromUtf8(isolate, person->uniqueId().c_str(), NewStringType::kNormal).ToLocalChecked());

	setNameObject(isolate, obj, person);
	Nan::Set(obj, New<String>("nickname").ToLocalChecked(),
		String::NewFromUtf8(isolate, person->nickname().c_str(), NewStringType::kNormal).ToLocalChecked());

	setOrganizationObject(isolate, obj, person);

	Nan::Set(obj, New<String>("note").ToLocalChecked(),
			 String::NewFromUtf8(isolate, person->note().c_str(), NewStringType::kNormal).ToLocalChecked());

	setAddressObject(isolate, obj, person);

	setVectorArray(isolate, obj, "emails", person->emails());
	setVectorArray(isolate, obj, "phoneNumbers", person->phoneNumbers());

	Nan::Set(obj, New<String>("image").ToLocalChecked(),
			 String::NewFromUtf8(isolate, person->image().c_str(), NewStringType::kNormal).ToLocalChecked());
}

class AddressBookWorker : public AsyncProgressWorker
{
  public:
	AddressBookWorker(Callback *callback, Callback *progress)
		: AsyncProgressWorker(callback), progress(progress), contacts() {}

	~AddressBookWorker() {}

	void Execute(const AsyncProgressWorker::ExecutionProgress &progress)
	{
		AddressBook ab;
		unsigned total = ab.contactCount();
		for (unsigned int i = 0; i < total; i++)
		{
			contacts.push_back(ab.getContact(i));
			int percent = ((double)i / (double)total) * 100;
			progress.Send(reinterpret_cast<const char *>(&percent), sizeof(int));
		}
	}

	void HandleProgressCallback(const char *data, size_t size)
	{
		Nan::HandleScope scope;

		v8::Local<v8::Value> argv[] = {
			New<v8::Integer>(*reinterpret_cast<int *>(const_cast<char *>(data)))};
		progress->Call(1, argv);
	}

	// We have the results, and we're back in the event loop.
	void HandleOKCallback()
	{
		Isolate *isolate = Isolate::GetCurrent();
		Nan::HandleScope scope;

		Local<Array> results = New<Array>(contacts.size());
		int i = 0;
		for_each(contacts.begin(), contacts.end(), [&](Person *person) {
			Local<Object> contact = Object::New(isolate);
			fillPersonObject(isolate, contact, person);
			Nan::Set(results, i, contact);
			i++;
		});

		Local<Value> argv[] = {results};
		callback->Call(1, argv);
	}

  private:
	Callback *progress;
	vector<Person *> contacts;
};

// Asynchronous access to the `getContacts()` function
NAN_METHOD(GetContacts)
{
	Callback *progress = new Callback(info[0].As<Function>());
	Callback *callback = new Callback(info[1].As<Function>());

	AsyncQueueWorker(new AddressBookWorker(callback, progress));
}

NAN_METHOD(GetMe)
{
	AddressBook ab;
	Isolate *isolate = Isolate::GetCurrent();

	Local<Object> me = Object::New(isolate);
	fillPersonObject(isolate, me, ab.getMe());

	info.GetReturnValue().Set(me);
}

NAN_METHOD(GetContact)
{
	int index = info[0].As<Uint32>()->Value();

	AddressBook ab;
	Isolate *isolate = Isolate::GetCurrent();

	Local<Object> contact = Object::New(isolate);
	fillPersonObject(isolate, contact, ab.getContact(index));

	info.GetReturnValue().Set(contact);
}

NAN_METHOD(GetContactsCount)
{
	AddressBook ab;
	info.GetReturnValue().Set((unsigned)ab.contactCount());
}

NAN_MODULE_INIT(Init)
{
	Nan::Set(target, New<String>("getMe").ToLocalChecked(),
			 GetFunction(New<FunctionTemplate>(GetMe)).ToLocalChecked());

	Nan::Set(target, New<String>("getContact").ToLocalChecked(),
			 GetFunction(New<FunctionTemplate>(GetContact)).ToLocalChecked());

	Nan::Set(target, New<String>("getContactsCount").ToLocalChecked(),
			 GetFunction(New<FunctionTemplate>(GetContactsCount)).ToLocalChecked());

	Nan::Set(target, New<String>("getContacts").ToLocalChecked(),
			 GetFunction(New<FunctionTemplate>(GetContacts)).ToLocalChecked());
}

NODE_MODULE(addon, Init)
