from protobuf3.message import Message
from protobuf3.fields import EnumField, BytesField, MessageField
from enum import Enum


class cust_srcp_device_begin(Message):

    class STAGE(Enum):
        BETA = 0
        QA = 1
        PROD = 2


class cust_srcp_device_csr(Message):
    pass


class cust_srcp_device_csr_status(Message):

    class STATUS(Enum):
        CS_SUCCESS = 0
        CS_ERROR = 1
        DEV_SUCCESS = 10
        DEV_TIMEOUT = 11
        DEV_FAILURE = 12


class cust_srcp_device_store_cred(Message):
    pass


class cust_srcp_pkt(Message):

    class TYPE(Enum):
        BEGIN = 0
        DEVICE_CSR = 1
        CSR_STATUS = 2
        STORE_CRED = 5

cust_srcp_device_begin.add_field('stage', EnumField(field_number=1, optional=True, enum_cls=cust_srcp_device_begin.STAGE))
cust_srcp_device_csr.add_field('smsn', BytesField(field_number=1, optional=True))
cust_srcp_device_csr.add_field('ed25519_puk', BytesField(field_number=2, optional=True))
cust_srcp_device_csr.add_field('p256r1_puk', BytesField(field_number=3, optional=True))
cust_srcp_device_csr.add_field('signature', BytesField(field_number=4, optional=True))
cust_srcp_device_csr.add_field('transaction_id', BytesField(field_number=5, optional=True))
cust_srcp_device_csr_status.add_field('transaction_id', BytesField(field_number=1, optional=True))
cust_srcp_device_csr_status.add_field('status', EnumField(field_number=2, optional=True, enum_cls=cust_srcp_device_csr_status.STATUS))
cust_srcp_device_csr_status.add_field('hash', BytesField(field_number=3, optional=True))
cust_srcp_device_csr_status.add_field('signature', BytesField(field_number=4, optional=True))
cust_srcp_device_store_cred.add_field('transaction_id', BytesField(field_number=1, optional=True))
cust_srcp_device_store_cred.add_field('ed25519_sig', BytesField(field_number=2, optional=True))
cust_srcp_device_store_cred.add_field('p256r1_sig', BytesField(field_number=3, optional=True))
cust_srcp_device_store_cred.add_field('signature', BytesField(field_number=4, optional=True))
cust_srcp_device_store_cred.add_field('attested_csr_hash', BytesField(field_number=5, optional=True))
cust_srcp_pkt.add_field('type', EnumField(field_number=1, optional=True, enum_cls=cust_srcp_pkt.TYPE))
cust_srcp_pkt.add_field('srcp_begin', MessageField(field_number=2, optional=True, message_cls=cust_srcp_device_begin))
cust_srcp_pkt.add_field('device_csr', MessageField(field_number=3, optional=True, message_cls=cust_srcp_device_csr))
cust_srcp_pkt.add_field('status', MessageField(field_number=4, optional=True, message_cls=cust_srcp_device_csr_status))
cust_srcp_pkt.add_field('store_cred', MessageField(field_number=7, optional=True, message_cls=cust_srcp_device_store_cred))
