for i in range(16):
    print("irq" + str(i) + ':')
    print("\tpush\t$" + str(hex(i)))
    print("\tpush\t$" + str(hex(i+32)))
    print("\tjmp\tirq_common")