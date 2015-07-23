package com.hsm.barcode;

public class SymbologyConfig{
	public SymbologyConfig(int symbologyID){
		this.symID = symbologyID;
		
	}
	public int symID;
	public int Mask;
	public int Flags;
	public int MinLength;
	public int MaxLength;
}


