#!/usr/bin/env ruby

require 'coap'
require 'msgpack'
require 'json'

url = 'coap://192.168.179.21/light'

response = CoAP::Client.new.get_by_uri(url).payload

obj = MessagePack.unpack(response);
puts obj.to_json

