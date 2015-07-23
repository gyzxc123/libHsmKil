package com.hsm.barcode;

public final class ExposureValues {
	private ExposureValues(){
		
	}

	public class Setting{
		public Setting(){
		}
		public static final int AutoExposureMode = 0;
		public static final int MethodToUse = 1;
		public static final int TargetValue = 2;
		public static final int TargetPercentile = 3;
		public static final int TargetAcceptanceGap = 4;
		public static final int MaximumExposure = 5;
		public static final int MaximumGain = 6;
		public static final int StartingExposure = 7;
		public static final int StartingGain = 8;
		public static final int FrameRate = 9;
		public static final int ImageMustConform = 10;
		public static final int TriesForConforming = 11;
		public static final int SpecularExclusion = 12;
		public static final int SpecialSpecularSaturation = 13;
		public static final int SpecialSpecularLimit = 14;
		public static final int AdjustExpWhenIllumOff = 15;
		public static final int FixedExposure = 16;
		public static final int FixedGain = 17;
		public static final int FixedFrameRate = 18;
		
	}	
	
	
	// Values used to set the auto exposure method that will be used during image capture
	public final static class Method{
		private Method(){
		}
		public static final int UNIFORM = 0;
	    public static final int CENTER_ONLY = 1;
	    public static final int CENTER_WEIGHTED = 2;
	}

	// Values used to set exposure specular exclusion settings in exposure settings class
	public final class SpecularExclusion {
		private SpecularExclusion(){
		}
		
		public static final int OFF = 0;   
		public static final int MINIMAL = 1;  
		public static final int MODERATE = 2;  
		public static final int AGGRESSIVE = 3;  
		public static final int SPECIAL = 4;  
	}
	
	
	// Values used to set the auto exposure mode that will be used during image capture
	public final static class AutoExposureMode{
		private AutoExposureMode(){
		}
		public static final int FIXED = 0;
		public static final int ON_CHIP = 1;
		public static final int HHP = 2;
		public static final int HHP_PRESENTATION = 3;
		public static final int CONTEXT_SENSITIVE = 4;
		public static final int OPENLOOP_GAIN = 5;
		public static final int CELLPHONE = 6;
		public static final int DUAL_TRACK = 7;
		public static final int CONTEXT_SENSITIVE_DUAL_TRACK = 8;
	}

}
