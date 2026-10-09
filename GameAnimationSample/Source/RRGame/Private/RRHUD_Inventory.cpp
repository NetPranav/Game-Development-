// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.
// Resident-Evil-2-style inventory: equipped quick slots + condition monitor on the left, a 4-wide pouch grid on the right.
// Mouse (hover / click / wheel) and keyboard (WASD, arrows, Enter, Backspace, Q/E) both work while the game is paused.

#include "RRHUD.h"
#include "RRGame.h"
#include "RRGameMode.h"
#include "RRLoadoutComponent.h"
#include "RRSuiBridge.h"
#include "AAAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

namespace
{
	float Iv_Ease(float T) { T = FMath::Clamp(T, 0.0f, 1.0f); return T * T * (3.0f - 2.0f * T); }
	float Iv_Fade(double T, double Start, double Dur) { return Iv_Ease((float)((T - Start) / FMath::Max(0.0001, Dur))); }

	const FLinearColor Iv_Amber(1.0f, 0.62f, 0.16f, 1.0f);
	const FLinearColor Iv_Cyan(0.45f, 0.85f, 1.0f, 1.0f);
	FLinearColor Iv_WithA(FLinearColor C, float A) { C.A = A; return C; }

	uint32 Iv_GearHash(const FRRGearData& G)
	{
		return G.Seed != 0 ? (uint32)(G.Seed ^ (G.Seed >> 32)) : GetTypeHash(G.Id);
	}

	FString Iv_KindName(ERRGearKind K)
	{
		return K == ERRGearKind::Weapon ? TEXT("WEAPON") : (K == ERRGearKind::Armor ? TEXT("ARMOR") : TEXT("ABILITY"));
	}

	int32 Iv_MainStat(const FRRGearData& G)
	{
		return G.Kind == ERRGearKind::Weapon ? G.Damage : (G.Kind == ERRGearKind::Armor ? G.Armor : G.Power);
	}

	FString Iv_StatLabel(const FRRGearData& G)
	{
		return G.Kind == ERRGearKind::Weapon ? TEXT("DMG") : (G.Kind == ERRGearKind::Armor ? TEXT("ARM") : TEXT("PWR"));
	}
}

void ARRHUD::DrawGearIcon(ERRGearKind Kind, int64 Seed, float X, float Y, float S, float Alpha)
{
	const uint32 H = Seed != 0 ? (uint32)(Seed ^ (Seed >> 32)) : 0u;
	const FLinearColor Accent = Iv_WithA(FLinearColor::MakeFromHSV8((uint8)(H & 0xFF), 150, 255), Alpha);
	const FLinearColor Iv_Steel(0.78f, 0.82f, 0.88f, Alpha);
	const FLinearColor Bright(0.96f, 0.98f, 1.0f, Alpha);
	const FLinearColor Dark(0.20f, 0.16f, 0.13f, Alpha);
	const float CX = X + S * 0.5f, CY = Y + S * 0.5f;

	if (Kind == ERRGearKind::Weapon)
	{
		const bool bThin = ((H >> 8) & 1u) != 0u;
		const FVector2D Dir(0.7071f, -0.7071f), N(0.7071f, 0.7071f);
		auto At = [&](float T) { return FVector2D(CX + Dir.X * T * S, CY + Dir.Y * T * S); };
		const float BladeW = (bThin ? 0.065f : 0.100f) * S;
		const FVector2D BC = At(0.12f);
		RotRect(BC.X, BC.Y, BladeW, 0.56f * S, 45.0f, Iv_Steel);
		RotRect(BC.X, BC.Y, BladeW * 0.32f, 0.56f * S, 45.0f, Bright);
		const FVector2D E = At(0.40f);
		const FVector2D Tip = FVector2D(E.X + Dir.X * 0.10f * S, E.Y + Dir.Y * 0.10f * S);
		Tri(FVector2D(E.X + N.X * BladeW * 0.5f, E.Y + N.Y * BladeW * 0.5f), FVector2D(E.X - N.X * BladeW * 0.5f, E.Y - N.Y * BladeW * 0.5f), Tip, Iv_Steel);
		const FVector2D G = At(-0.17f);
		RotRect(G.X, G.Y, 0.31f * S, 0.055f * S, 45.0f, Accent);
		const FVector2D Gr = At(-0.27f);
		RotRect(Gr.X, Gr.Y, 0.045f * S, 0.16f * S, 45.0f, Dark);
		const FVector2D Pm = At(-0.37f);
		RotRect(Pm.X, Pm.Y, 0.075f * S, 0.075f * S, 45.0f, Accent);
	}
	else if (Kind == ERRGearKind::Armor)
	{
		const float R = S * 0.46f;
		const FVector2D Pts[7] = { {-0.62f, -0.62f}, {0.62f, -0.62f}, {0.78f, -0.02f}, {0.44f, 0.68f}, {0.0f, 0.92f}, {-0.44f, 0.68f}, {-0.78f, -0.02f} };
		const FVector2D C0(CX, CY - R * 0.10f);
		auto Fan = [&](float Sc, const FLinearColor& Col)
		{
			for (int32 i = 0; i < 7; ++i)
			{
				const FVector2D A(C0.X + Pts[i].X * R * Sc, C0.Y + Pts[i].Y * R * Sc);
				const FVector2D B(C0.X + Pts[(i + 1) % 7].X * R * Sc, C0.Y + Pts[(i + 1) % 7].Y * R * Sc);
				Tri(C0, A, B, Col);
			}
		};
		Fan(1.0f, FLinearColor(0.34f, 0.37f, 0.41f, Alpha));
		Fan(0.78f, FLinearColor(0.55f, 0.60f, 0.66f, Alpha));
		Fan(0.40f, FLinearColor(0.30f, 0.33f, 0.37f, Alpha));
		Box(CX - R * 0.62f, CY - R * 0.30f, R * 1.24f, S * 0.045f, Accent);
		Box(CX - S * 0.012f, CY - R * 0.55f, S * 0.024f, R * 1.15f, FLinearColor(0.1f, 0.11f, 0.12f, Alpha));
	}
	else
	{
		const float R = S * 0.42f;
		auto Octa = [&](float Sc, const FLinearColor& Col)
		{
			for (int32 i = 0; i < 8; ++i)
			{
				const float A0 = i * PI * 0.25f + PI * 0.125f, A1 = (i + 1) * PI * 0.25f + PI * 0.125f;
				Tri(FVector2D(CX, CY), FVector2D(CX + FMath::Cos(A0) * R * Sc, CY + FMath::Sin(A0) * R * Sc), FVector2D(CX + FMath::Cos(A1) * R * Sc, CY + FMath::Sin(A1) * R * Sc), Col);
			}
		};
		Octa(1.0f, Iv_WithA(Accent, Alpha * 0.55f));
		Octa(0.80f, FLinearColor(0.07f, 0.08f, 0.10f, Alpha));
		auto P = [&](float PX, float PY) { return FVector2D(CX + PX * S, CY + PY * S); };
		Tri(P(0.10f, -0.30f), P(-0.12f, 0.04f), P(0.02f, 0.04f), Bright);
		Tri(P(-0.02f, -0.04f), P(0.12f, -0.04f), P(-0.08f, 0.30f), Bright);
	}
}

void ARRHUD::DrawItemIcon(const FRRInventoryItem& Item, float X, float Y, float S, float Alpha)
{
	FRRGearData G = Item.Gear;
	if (G.Seed == 0)
	{
		G.Seed = (int64)GetTypeHash(G.Id) * 2654435761LL; // stable look for local items
	}
	DrawGearIcon(G.Kind, G.Seed, X, Y, S, Alpha);
}

void ARRHUD::DrawInventoryWallet(float PX, float PY, float PW, float PH)
{
	URRSuiBridge* Sui = GetGameInstance() ? GetGameInstance()->GetSubsystem<URRSuiBridge>() : nullptr;
	const bool bConn = Sui && Sui->IsConnected();

	Box(PX, PY, PW, PH, FLinearColor(0.05f, 0.055f, 0.06f, 0.9f));
	Frame(PX, PY, PW, PH, FMath::Max(1.0f, U(1.5f)), FLinearColor(0.7f, 0.74f, 0.8f, 0.3f));
	Brackets(PX - U(6.0f), PY - U(6.0f), PW + U(12.0f), PH + U(12.0f), U(24.0f), FMath::Max(2.0f, U(2.5f)), Iv_Amber);

	float Y = PY + U(34.0f);
	const float LX = PX + U(44.0f), RX = PX + PW - U(44.0f);
	Txt(TEXT("SUI WALLET"), LX, Y, 40.0f, FLinearColor::White, 0.0f, 8.0f);
	Y += U(86.0f);
	auto Row = [&](const TCHAR* Key, const FString& Val, const FLinearColor& Col)
	{
		Txt(Key, LX, Y, 22.0f, FLinearColor(0.55f, 0.58f, 0.62f, 1.0f), 0.0f, 4.0f);
		Txt(Val, RX, Y - U(3.0f), 28.0f, Col, 1.0f, 1.5f);
		Box(LX, Y + U(38.0f), PW - U(88.0f), 1.0f, FLinearColor(1, 1, 1, 0.1f));
		Y += U(52.0f);
	};
	Row(TEXT("NETWORK"), TEXT("SUI TESTNET"), FLinearColor(0.8f, 0.85f, 0.95f, 1.0f));
	Row(TEXT("STATUS"), bConn ? TEXT("CONNECTED") : TEXT("NOT CONNECTED"), bConn ? FLinearColor(0.3f, 0.95f, 0.55f, 1.0f) : FLinearColor(0.95f, 0.38f, 0.35f, 1.0f));
	Row(TEXT("ADDRESS"), bConn ? ShortAddress(Sui->GetWalletAddress()) : FString(TEXT("-")), FLinearColor(0.85f, 0.88f, 0.92f, 1.0f));
	Row(TEXT("CREDITS"), bConn ? FString::FromInt(Sui->GetCredits()) : FString(TEXT("-")), Iv_Amber);
	Row(TEXT("GEAR NFTS"), bConn ? FString::FromInt(Sui->GetOwnedGear().Num()) : FString(TEXT("-")), Iv_Cyan);

	// two buttons
	const float BW = (PW - U(88.0f) - U(24.0f)) * 0.5f;
	const float BH = U(64.0f);
	const float BY = PY + PH - BH - U(36.0f);
	for (int32 b = 0; b < 2; ++b)
	{
		const float BX = LX + b * (BW + U(24.0f));
		const bool bHover = Hit(BX, BY, BW, BH);
		Box(BX, BY, BW, BH, bHover ? FLinearColor(1.0f, 0.62f, 0.16f, 0.25f) : FLinearColor(1, 1, 1, 0.06f));
		Frame(BX, BY, BW, BH, FMath::Max(1.0f, U(1.5f)), bHover ? Iv_Amber : FLinearColor(1, 1, 1, 0.3f));
		Txt(b == 0 ? TEXT("CONNECT WALLET  (F2)") : TEXT("REFRESH GEAR  (F3)"), BX + BW * 0.5f, BY + U(16.0f), 26.0f, FLinearColor::White, 0.5f, 3.0f);
		if (bHover && bClickL && Sui)
		{
			bClickL = false;
			if (b == 0) Sui->BeginWalletConnect(); else Sui->FetchGear();
		}
	}
}

void ARRHUD::DrawInventory()
{
	APlayerController* PC = GetOwningPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	URRLoadoutComponent* Loadout = Pawn ? Pawn->FindComponentByClass<URRLoadoutComponent>() : nullptr;

	const double T = Now - InvOpenTime;
	const float In = Iv_Fade(T, 0.0, 0.25);

	// ---------------- backdrop ----------------
	Box(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(0.012f, 0.014f, 0.017f, 0.97f));
	VGrad(0.0f, 0.0f, CanvasW, CanvasH, FLinearColor(0.07f, 0.08f, 0.09f, 0.55f), FLinearColor(0.0f, 0.0f, 0.0f, 0.0f), 24);
	const float LineStep = FMath::Max(4.0f, U(5.0f));
	for (float Yl = 0.0f; Yl < CanvasH; Yl += LineStep)
	{
		Box(0.0f, Yl, CanvasW, 1.0f, FLinearColor(1, 1, 1, 0.012f));
	}
	Vignette(0.75f);

	const float MX = CanvasW * 0.06f;
	const float MY = CanvasH * 0.055f;
	const float CT = MY + U(112.0f);
	const float CB = CanvasH - U(92.0f);

	// ---------------- header ----------------
	Txt(TEXT("INVENTORY"), MX, MY, 58.0f, FLinearColor(0.94f, 0.95f, 0.97f, In), 0.0f, 12.0f);
	Txt(TEXT("KAIZEEN  //  FIELD KIT"), MX + U(4.0f), MY + U(66.0f), 18.0f, Iv_WithA(Iv_Amber, In), 0.0f, 8.0f);
	Box(MX, MY + U(98.0f), CanvasW - MX * 2.0f, FMath::Max(1.0f, U(1.5f)), FLinearColor(1, 1, 1, 0.22f * In));
	Box(MX, MY + U(98.0f), U(180.0f), FMath::Max(2.0f, U(3.0f)), Iv_WithA(Iv_Amber, In));

	// tabs (top right)
	{
		const TCHAR* Tabs[2] = { TEXT("ITEMS"), TEXT("WALLET") };
		float TX = CanvasW - MX;
		for (int32 i = 1; i >= 0; --i)
		{
			const float W = TxtWidth(Tabs[i], 30.0f, 6.0f);
			TX -= W;
			const float TY = MY + U(22.0f);
			const bool bHover = Hit(TX - U(10.0f), TY - U(6.0f), W + U(20.0f), U(46.0f));
			if (bHover && bClickL) { ActiveTab = i; bClickL = false; }
			const bool bAct = (ActiveTab == i);
			Txt(Tabs[i], TX, TY, 30.0f, bAct ? FLinearColor(1, 1, 1, In) : FLinearColor(0.5f, 0.53f, 0.57f, In), 0.0f, 6.0f);
			if (bAct) Box(TX, TY + U(44.0f), W, FMath::Max(2.0f, U(3.0f)), Iv_WithA(Iv_Amber, In));
			TX -= U(52.0f);
		}
		if (KeyPressed(EKeys::Q)) ActiveTab = 0;
		if (KeyPressed(EKeys::E)) ActiveTab = 1;
	}

	if (!Loadout)
	{
		Txt(TEXT("NO LOADOUT DATA"), CanvasW * 0.5f, CanvasH * 0.5f, 40.0f, FLinearColor(0.9f, 0.3f, 0.3f, 1.0f), 0.5f, 6.0f);
		return;
	}

	// ---------------- footer: key hints ----------------
	{
		float FX = MX;
		const float FY = CanvasH - U(64.0f);
		auto Cap = [&](const TCHAR* Key, const TCHAR* Label)
		{
			const float KW = TxtWidth(Key, 18.0f, 1.0f) + U(20.0f);
			Box(FX, FY - U(4.0f), KW, U(32.0f), FLinearColor(1, 1, 1, 0.10f));
			Frame(FX, FY - U(4.0f), KW, U(32.0f), 1.0f, FLinearColor(1, 1, 1, 0.35f));
			Txt(Key, FX + KW * 0.5f, FY, 18.0f, FLinearColor::White, 0.5f, 1.0f, false);
			FX += KW + U(10.0f);
			Txt(Label, FX, FY, 20.0f, FLinearColor(0.66f, 0.69f, 0.73f, 1.0f), 0.0f, 2.0f);
			FX += TxtWidth(Label, 20.0f, 2.0f) + U(34.0f);
		};
		Cap(TEXT("CLICK / ENTER"), TEXT("EQUIP"));
		Cap(TEXT("RMB / BKSP"), TEXT("UNEQUIP"));
		Cap(TEXT("WASD"), TEXT("MOVE"));
		Cap(TEXT("Q  E"), TEXT("TABS"));
		Cap(TEXT("TAB / ESC"), TEXT("CLOSE"));
	}

	if (ActiveTab == 1)
	{
		DrawInventoryWallet(CanvasW * 0.25f, CT, CanvasW * 0.5f, CB - CT - U(8.0f));
		return;
	}

	// ---------------- data ----------------
	TArray<FRRInventoryItem> Inv = Loadout->BuildInventory();
	FRRGearData Eq[3];
	bool bEq[3] = { false, false, false };
	const ERRGearKind Kinds[3] = { ERRGearKind::Weapon, ERRGearKind::Armor, ERRGearKind::Ability };
	for (int32 k = 0; k < 3; ++k) bEq[k] = Loadout->GetEquipped(Kinds[k], Eq[k]);
	auto IsEquipped = [&](const FRRGearData& G)
	{
		for (int32 k = 0; k < 3; ++k) { if (bEq[k] && Eq[k].Kind == G.Kind && Eq[k].Id == G.Id) return true; }
		return false;
	};

	const float LX = MX;
	const float LW = CanvasW * 0.385f;
	const float GX0 = LX + LW + CanvasW * 0.045f;
	const float GW0 = CanvasW - MX - GX0;

	// ================= LEFT: equipped slots =================
	Txt(TEXT("EQUIPPED"), LX, CT, 24.0f, FLinearColor(0.7f, 0.73f, 0.78f, In), 0.0f, 7.0f);
	Box(LX, CT + U(34.0f), LW, 1.0f, FLinearColor(1, 1, 1, 0.2f));

	const float SlotGap = U(12.0f);
	const float SlotH = FMath::Min(U(112.0f), (CB - CT) * 0.155f);
	float SY = CT + U(48.0f);
	int32 HoverSlot = -1;
	for (int32 k = 0; k < 3; ++k)
	{
		const bool bHover = Hit(LX, SY, LW, SlotH);
		if (bHover) HoverSlot = k;
		Box(LX, SY, LW, SlotH, bHover ? FLinearColor(0.15f, 0.15f, 0.14f, 0.95f) : FLinearColor(0.085f, 0.095f, 0.105f, 0.95f));
		Frame(LX, SY, LW, SlotH, 1.0f, bEq[k] ? Iv_WithA(Iv_Amber, 0.75f) : FLinearColor(1, 1, 1, 0.22f));

		const float Ico = SlotH - U(16.0f);
		Box(LX + U(8.0f), SY + U(8.0f), Ico, Ico, FLinearColor(0.03f, 0.035f, 0.04f, 1.0f));
		// number tab
		Tri(FVector2D(LX + U(8.0f), SY + U(8.0f)), FVector2D(LX + U(8.0f) + Ico * 0.34f, SY + U(8.0f)), FVector2D(LX + U(8.0f), SY + U(8.0f) + Ico * 0.34f), FLinearColor(0.9f, 0.9f, 0.9f, 0.9f));
		Txt(FString::FromInt(k + 1), LX + U(11.0f), SY + U(8.0f), 17.0f, FLinearColor(0.05f, 0.05f, 0.05f, 1.0f), 0.0f, 0.0f, false);

		const float TX = LX + Ico + U(30.0f);
		Txt(Iv_KindName(Kinds[k]), TX, SY + U(10.0f), 18.0f, FLinearColor(0.5f, 0.53f, 0.57f, 1.0f), 0.0f, 5.0f);
		if (bEq[k])
		{
			FRRInventoryItem Tmp; Tmp.Gear = Eq[k];
			DrawItemIcon(Tmp, LX + U(8.0f) + Ico * 0.05f, SY + U(8.0f) + Ico * 0.05f, Ico * 0.9f, 1.0f);
			Txt(Eq[k].Name.ToUpper(), TX, SY + U(34.0f), 28.0f, FLinearColor::White, 0.0f, 2.0f);
			Txt(FString::Printf(TEXT("%s %d    LV %d"), *Iv_StatLabel(Eq[k]), Iv_MainStat(Eq[k]), Eq[k].Level), TX, SY + U(70.0f), 22.0f, Iv_Cyan, 0.0f, 2.0f);
		}
		else
		{
			const FLinearColor X(0.35f, 0.37f, 0.4f, 0.8f);
			Line(LX + U(8.0f) + Ico * 0.2f, SY + U(8.0f) + Ico * 0.2f, LX + U(8.0f) + Ico * 0.8f, SY + U(8.0f) + Ico * 0.8f, 1.5f, X);
			Line(LX + U(8.0f) + Ico * 0.8f, SY + U(8.0f) + Ico * 0.2f, LX + U(8.0f) + Ico * 0.2f, SY + U(8.0f) + Ico * 0.8f, 1.5f, X);
			Txt(TEXT("EMPTY"), TX, SY + U(40.0f), 26.0f, FLinearColor(0.35f, 0.37f, 0.4f, 1.0f), 0.0f, 4.0f);
		}
		if (bHover && (bClickL || bClickR) && bEq[k])
		{
			Loadout->Unequip(Kinds[k]);
			bClickL = bClickR = false;
		}
		SY += SlotH + SlotGap;
	}

	// ================= LEFT: condition monitor (RE2 homage) =================
	float CondY = SY + U(10.0f);
	{
		float Hp = 100.0f, MaxHp = 100.0f;
		if (Pawn)
		{
			UAbilitySystemComponent* ASC = nullptr;
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn)) ASC = ASI->GetAbilitySystemComponent();
			else if (IAbilitySystemInterface* PSI = Cast<IAbilitySystemInterface>(Pawn->GetPlayerState())) ASC = PSI->GetAbilitySystemComponent();
			if (ASC)
			{
				Hp = ASC->GetNumericAttribute(UAAAAttributeSet::GetHealthAttribute());
				MaxHp = FMath::Max(1.0f, ASC->GetNumericAttribute(UAAAAttributeSet::GetMaxHealthAttribute()));
			}
		}
		const float Frac = FMath::Clamp(Hp / MaxHp, 0.0f, 1.0f);
		const bool bFine = Frac > 0.66f, bCaution = Frac > 0.33f;
		const FLinearColor Col = bFine ? FLinearColor(0.25f, 0.95f, 0.5f, 1.0f) : (bCaution ? FLinearColor(1.0f, 0.82f, 0.2f, 1.0f) : FLinearColor(1.0f, 0.25f, 0.2f, 1.0f));
		const TCHAR* Word = bFine ? TEXT("FINE") : (bCaution ? TEXT("CAUTION") : TEXT("DANGER"));

		const float CH = FMath::Min(U(104.0f), CB - CondY - U(150.0f));
		if (CH > U(50.0f))
		{
			Box(LX, CondY, LW, CH, FLinearColor(0.03f, 0.04f, 0.045f, 0.95f));
			Frame(LX, CondY, LW, CH, 1.0f, Iv_WithA(Col, 0.55f));
			Txt(Word, LX + U(22.0f), CondY + CH * 0.5f - U(24.0f), 38.0f, Col, 0.0f, 5.0f);
			Txt(FString::Printf(TEXT("HP %d / %d"), FMath::RoundToInt(Hp), FMath::RoundToInt(MaxHp)), LX + U(24.0f), CondY + CH - U(30.0f), 18.0f, FLinearColor(0.6f, 0.63f, 0.67f, 1.0f), 0.0f, 3.0f);

			// animated ECG
			const float EX = LX + LW * 0.42f, EW = LW * 0.54f, EY = CondY + CH * 0.5f, Amp = CH * 0.36f;
			const float Speed = bFine ? 0.55f : (bCaution ? 0.85f : 1.35f);
			const int32 Seg = 56;
			FVector2D Prev(EX, EY);
			for (int32 i = 1; i <= Seg; ++i)
			{
				const float Xn = (float)i / Seg;
				const float Ph = FMath::Frac(Xn * 2.0f - (float)Now * Speed);
				float V = 0.0f;
				if (Ph > 0.10f && Ph < 0.16f) V = 0.18f * FMath::Sin((Ph - 0.10f) / 0.06f * PI);
				else if (Ph > 0.22f && Ph < 0.25f) V = -0.25f * ((Ph - 0.22f) / 0.03f);
				else if (Ph >= 0.25f && Ph < 0.30f) V = -0.25f + 1.25f * ((Ph - 0.25f) / 0.05f);
				else if (Ph >= 0.30f && Ph < 0.35f) V = 1.0f - 1.35f * ((Ph - 0.30f) / 0.05f);
				else if (Ph >= 0.35f && Ph < 0.38f) V = -0.35f + 0.35f * ((Ph - 0.35f) / 0.03f);
				else if (Ph > 0.50f && Ph < 0.62f) V = 0.14f * FMath::Sin((Ph - 0.50f) / 0.12f * PI);
				const FVector2D Cur(EX + EW * Xn, EY - V * Amp);
				const float Fall = FMath::Clamp(Xn * 1.6f, 0.0f, 1.0f); // trail fades in from the left
				Line(Prev.X, Prev.Y, Cur.X, Cur.Y, FMath::Max(1.5f, U(2.2f)), Iv_WithA(Col, 0.15f + 0.85f * Fall));
				Prev = Cur;
			}
			CondY += CH + U(16.0f);
		}
	}

	// ================= RIGHT: pouch grid =================
	const int32 Cols = 4;
	const int32 VisRows = 5;
	const int32 NumItems = Inv.Num();
	const int32 TotalCells = FMath::Max(Cols * VisRows, FMath::DivideAndRoundUp(NumItems, Cols) * Cols);
	const int32 TotalRows = TotalCells / Cols;
	const float Gap = U(10.0f);
	const float GridTop = CT + U(48.0f);
	const float GridAvailH = CB - GridTop;
	const float CellS = FMath::Max(U(60.0f), FMath::Min((GW0 - Gap * (Cols - 1) - U(24.0f)) / Cols, (GridAvailH - Gap * (VisRows - 1)) / VisRows));
	const float GridW = CellS * Cols + Gap * (Cols - 1);
	const float GX = GX0 + FMath::Max(0.0f, (GW0 - GridW - U(24.0f)) * 0.5f);

	Txt(TEXT("POUCH"), GX, CT, 24.0f, FLinearColor(0.7f, 0.73f, 0.78f, In), 0.0f, 7.0f);
	Txt(FString::Printf(TEXT("%d / %d"), NumItems, TotalCells), GX + GridW, CT, 24.0f, FLinearColor(0.7f, 0.73f, 0.78f, In), 1.0f, 3.0f);
	Box(GX, CT + U(34.0f), GridW, 1.0f, FLinearColor(1, 1, 1, 0.2f));

	// navigation
	if (NumItems > 0)
	{
		if (bLeft) InvSel = FMath::Max(0, InvSel - 1);
		if (bRight) InvSel = FMath::Min(TotalCells - 1, InvSel + 1);
		if (bUp) InvSel = FMath::Max(0, InvSel - Cols);
		if (bDown) InvSel = FMath::Min(TotalCells - 1, InvSel + Cols);
	}
	InvSel = FMath::Clamp(InvSel, 0, TotalCells - 1);
	{
		const int32 Row = InvSel / Cols;
		if (Row < InvScrollRow) InvScrollRow = Row;
		if (Row >= InvScrollRow + VisRows) InvScrollRow = Row - VisRows + 1;
	}
	InvScrollRow = FMath::Clamp(InvScrollRow, 0, FMath::Max(0, TotalRows - VisRows));

	const bool bMouseMoved = (FMath::Abs(MouseX - LastMouseX) + FMath::Abs(MouseY - LastMouseY)) > 0.5f;
	LastMouseX = MouseX; LastMouseY = MouseY;

	int32 HoverCell = -1;
	for (int32 r = 0; r < VisRows; ++r)
	{
		for (int32 c = 0; c < Cols; ++c)
		{
			const int32 Idx = (InvScrollRow + r) * Cols + c;
			if (Idx >= TotalCells) continue;
			const float CX = GX + c * (CellS + Gap);
			const float CY = GridTop + r * (CellS + Gap);
			const bool bHover = Hit(CX, CY, CellS, CellS);
			if (bHover) { HoverCell = Idx; if (bMouseMoved) InvSel = Idx; }
			const bool bSelected = (InvSel == Idx);
			const float Cell = Iv_Fade(T, 0.02 * (r * Cols + c), 0.25); // cells appear in a quick cascade

			if (Idx < NumItems)
			{
				const FRRInventoryItem& It = Inv[Idx];
				const bool bE = IsEquipped(It.Gear);
				Box(CX, CY, CellS, CellS, bE ? FLinearColor(0.16f, 0.125f, 0.065f, 0.96f * Cell) : FLinearColor(0.105f, 0.115f, 0.125f, 0.96f * Cell));
				Frame(CX, CY, CellS, CellS, 1.0f, bE ? Iv_WithA(Iv_Amber, 0.8f * Cell) : FLinearColor(1, 1, 1, 0.22f * Cell));
				DrawItemIcon(It, CX + CellS * 0.14f, CY + CellS * 0.12f, CellS * 0.72f, Cell);

				// corner tab: E (equipped) or the 1-9 hotkey
				const float Tab = CellS * 0.30f;
				Tri(FVector2D(CX, CY), FVector2D(CX + Tab, CY), FVector2D(CX, CY + Tab), bE ? Iv_WithA(Iv_Amber, Cell) : FLinearColor(0.88f, 0.88f, 0.88f, 0.9f * Cell));
				if (bE) Txt(TEXT("E"), CX + CellS * 0.035f, CY + CellS * 0.005f, 20.0f, FLinearColor(0.08f, 0.05f, 0.0f, Cell), 0.0f, 0.0f, false);
				else if (Idx < 9) Txt(FString::FromInt(Idx + 1), CX + CellS * 0.035f, CY + CellS * 0.005f, 20.0f, FLinearColor(0.05f, 0.05f, 0.05f, Cell), 0.0f, 0.0f, false);

				if (It.bOnChain)
				{
					const float NW = CellS * 0.36f;
					Box(CX + CellS - NW, CY, NW, CellS * 0.17f, FLinearColor(0.1f, 0.45f, 0.75f, 0.95f * Cell));
					Txt(TEXT("NFT"), CX + CellS - NW * 0.5f, CY + CellS * 0.005f, 15.0f, FLinearColor(1, 1, 1, Cell), 0.5f, 1.0f, false);
				}
				Txt(FString::FromInt(Iv_MainStat(It.Gear)), CX + CellS - U(8.0f), CY + CellS - U(32.0f), 28.0f, FLinearColor(0.55f, 1.0f, 0.65f, Cell), 1.0f, 0.0f);
				Txt(FString::Printf(TEXT("LV%d"), It.Gear.Level), CX + U(8.0f), CY + CellS - U(26.0f), 16.0f, FLinearColor(0.6f, 0.63f, 0.67f, Cell), 0.0f, 1.0f, false);
			}
			else
			{
				Box(CX, CY, CellS, CellS, FLinearColor(0.045f, 0.05f, 0.055f, 0.85f * Cell));
				Frame(CX, CY, CellS, CellS, 1.0f, FLinearColor(1, 1, 1, 0.10f * Cell));
				const FLinearColor X(0.30f, 0.32f, 0.35f, 0.75f * Cell);
				Line(CX + CellS * 0.12f, CY + CellS * 0.12f, CX + CellS * 0.88f, CY + CellS * 0.88f, 1.2f, X);
				Line(CX + CellS * 0.88f, CY + CellS * 0.12f, CX + CellS * 0.12f, CY + CellS * 0.88f, 1.2f, X);
			}

			if (bSelected)
			{
				const float Pulse = 0.5f + 0.5f * FMath::Sin((float)Now * 5.0f);
				Box(CX, CY, CellS, CellS, FLinearColor(1, 1, 1, 0.05f + 0.05f * Pulse));
				Frame(CX - 1.0f, CY - 1.0f, CellS + 2.0f, CellS + 2.0f, FMath::Max(2.0f, U(2.5f)), FLinearColor(1, 1, 1, 0.95f));
				Brackets(CX - U(5.0f), CY - U(5.0f), CellS + U(10.0f), CellS + U(10.0f), CellS * 0.22f, FMath::Max(2.0f, U(2.5f)), Iv_WithA(Iv_Amber, 0.6f + 0.4f * Pulse));
			}
		}
	}

	// scrollbar
	if (TotalRows > VisRows)
	{
		const float SBX = GX + GridW + U(12.0f);
		const float SBH = VisRows * CellS + (VisRows - 1) * Gap;
		Box(SBX, GridTop, U(4.0f), SBH, FLinearColor(1, 1, 1, 0.1f));
		const float Th = SBH * VisRows / TotalRows;
		const float Frac = (float)InvScrollRow / (TotalRows - VisRows);
		Box(SBX, GridTop + (SBH - Th) * Frac, U(4.0f), Th, Iv_WithA(Iv_Amber, 0.9f));
	}

	// ================= actions =================
	const bool bSelFilled = Inv.IsValidIndex(InvSel);
	if (bSelFilled)
	{
		const FRRInventoryItem& Sel = Inv[InvSel];
		if (HoverCell == InvSel && bClickL)
		{
			Loadout->Equip(Sel.Gear);
			bClickL = false;
		}
		else if (bConfirm)
		{
			if (IsEquipped(Sel.Gear)) Loadout->Unequip(Sel.Gear.Kind); else Loadout->Equip(Sel.Gear);
		}
		if ((HoverCell == InvSel && bClickR) || KeyPressed(EKeys::BackSpace) || KeyPressed(EKeys::X))
		{
			Loadout->Unequip(Sel.Gear.Kind);
		}
	}
	(void)HoverSlot;

	// ================= LEFT: details of the selected / hovered item =================
	{
		const float DY = CondY + U(4.0f);
		const float DH = CB - DY;
		if (DH > U(90.0f))
		{
			Box(LX, DY, LW, DH, FLinearColor(0.045f, 0.05f, 0.055f, 0.92f));
			Frame(LX, DY, LW, DH, 1.0f, FLinearColor(1, 1, 1, 0.18f));
			if (bSelFilled)
			{
				const FRRGearData& G = Inv[InvSel].Gear;
				const bool bChain = Inv[InvSel].bOnChain;
				float Y = DY + U(14.0f);
				Txt(G.Name.ToUpper(), LX + U(20.0f), Y, 30.0f, FLinearColor::White, 0.0f, 2.0f);
				Y += U(40.0f);
				Txt(FString::Printf(TEXT("%s  -  %s"), *Iv_KindName(G.Kind), bChain ? TEXT("ON-CHAIN NFT") : TEXT("STARTER KIT")), LX + U(20.0f), Y, 18.0f, bChain ? Iv_Cyan : FLinearColor(0.6f, 0.63f, 0.67f, 1.0f), 0.0f, 3.0f);
				Y += U(32.0f);

				const TCHAR* Flavor = G.Kind == ERRGearKind::Weapon ? TEXT("Edge-tuned alloy blade. Click once, twice, three times: the third cut is a finisher.")
					: (G.Kind == ERRGearKind::Armor ? TEXT("Layered plating that soaks part of every hit and steadies your poise.")
						: TEXT("Power-core module. Strengthens your strikes while installed."));
				// word wrap
				FString Cur;
				TArray<FString> Words;
				FString(Flavor).ParseIntoArray(Words, TEXT(" "));
				const float MaxW = LW - U(40.0f);
				for (const FString& W : Words)
				{
					const FString Try = Cur.IsEmpty() ? W : Cur + TEXT(" ") + W;
					if (TxtWidth(Try, 19.0f, 0.5f) > MaxW && !Cur.IsEmpty())
					{
						if (Y + U(24.0f) < CB - U(8.0f)) Txt(Cur, LX + U(20.0f), Y, 19.0f, FLinearColor(0.72f, 0.75f, 0.8f, 1.0f), 0.0f, 0.5f);
						Y += U(24.0f);
						Cur = W;
					}
					else
					{
						Cur = Try;
					}
				}
				if (!Cur.IsEmpty() && Y + U(24.0f) < CB - U(8.0f)) { Txt(Cur, LX + U(20.0f), Y, 19.0f, FLinearColor(0.72f, 0.75f, 0.8f, 1.0f), 0.0f, 0.5f); }
				Y += U(36.0f);

				auto Stat = [&](const TCHAR* Label, int32 Value)
				{
					if (Y + U(26.0f) > CB - U(6.0f)) return;
					Txt(Label, LX + U(20.0f), Y, 18.0f, FLinearColor(0.5f, 0.53f, 0.57f, 1.0f), 0.0f, 4.0f);
					Txt(FString::FromInt(Value), LX + LW - U(20.0f), Y - U(2.0f), 22.0f, FLinearColor(0.55f, 1.0f, 0.65f, 1.0f), 1.0f, 1.0f);
					Y += U(28.0f);
				};
				if (G.Kind == ERRGearKind::Weapon) Stat(TEXT("DAMAGE"), G.Damage);
				if (G.Kind == ERRGearKind::Armor) Stat(TEXT("ARMOR"), G.Armor);
				if (G.Kind == ERRGearKind::Ability) Stat(TEXT("POWER"), G.Power);
				Stat(TEXT("LEVEL"), G.Level);
				if (bChain) Stat(TEXT("WINS"), G.Wins);
			}
			else
			{
				Txt(TEXT("EMPTY SLOT"), LX + LW * 0.5f, DY + DH * 0.5f - U(14.0f), 26.0f, FLinearColor(0.35f, 0.37f, 0.4f, 1.0f), 0.5f, 6.0f);
			}
		}
	}
}
