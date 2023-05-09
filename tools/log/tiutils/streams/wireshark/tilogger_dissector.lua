local logger_protocol = Proto("TILogger",  "TILogger Protocol")

pf_alias  =     ProtoField.string("tilogger.alias", "Alias", base.STRING)
pf_ts_local  =  ProtoField.string("tilogger.ts_local", "Device time", base.STRING)
pf_opcode =     ProtoField.string("tilogger.opcode", "Opcode", base.STRING)
pf_module =     ProtoField.string("tilogger.module", "Module", base.STRING)
pf_loglevel =   ProtoField.string("tilogger.level", "Log Level", base.STRING)
pf_file =       ProtoField.string("tilogger.file", "File", base.STRING)
pf_line =       ProtoField.string("tilogger.line", "Line", base.STRING)
pf_string =     ProtoField.string("tilogger.string", "String", base.STRING)

logger_protocol.fields = {
  pf_alias,
  pf_ts_local,
  pf_opcode,
  pf_module,
  pf_loglevel,
  pf_file,
  pf_line,
  pf_string
}


function split (inputstr)
	local t={}
	for str in string.gmatch(inputstr, "([^||]+)") do
		table.insert(t, str)
	end
	return t
end


function logger_protocol.dissector(buffer, pinfo, tree)
  length = buffer:len()
  if length == 0 then return end

  pinfo.cols.protocol = logger_protocol.name

  local subtree = tree:add(logger_protocol, buffer(), "TILogger")
  local stringbuf = buffer(0, length):string(ENC_UTF_8)

  parts = split(stringbuf)
  subtree:add(pf_alias, parts[1])
  subtree:add(pf_ts_local, parts[2])
  subtree:add(pf_opcode, parts[3])
  subtree:add(pf_module, parts[4])
  subtree:add(pf_loglevel, parts[5])
  subtree:add(pf_file, parts[6])
  subtree:add(pf_line, parts[7])
  subtree:add(pf_string, parts[8])
end

local raw_ip = DissectorTable.get("wtap_encap")
raw_ip:add(7, logger_protocol)
