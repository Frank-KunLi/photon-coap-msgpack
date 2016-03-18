#!/usr/bin/env ruby

require 'coap'
require 'msgpack'
require 'json'

state = "off"
if ARGV.size > 0
	state = ARGV[0]
end

url = 'coap://192.168.179.21/light'

msg = {
	"value" => {
		"id" => "light", 
		"v" => state
	}
}
puts "Sending msg to #{url}"
puts msg.to_json

response = CoAP::Client.new.put_by_uri(url,msg.to_msgpack)

