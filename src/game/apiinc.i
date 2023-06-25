.def api_CreateSprite $50 + (3 * 0) ;A sprite type, return sprite ID on A, creates a new sprite
.def api_MoveSprite $50 + (3 * 1) ;A sprite ID, HL position, sets sprite position
.def api_SetFrameFlip $50 + (3 * 2) ;A sprite ID, B frame, C flipped
.def api_RemoveSprite $50 + (3 * 3) ; A sprite id, destroy sprite
.def api_CreateDot $50 + (3 * 4) ; HL position, A dot type ;Returns dot ID in A 
.def api_UpdateDot $50 + (3 * 5) ; HL position, A box ID ;Returns dot type in A 
.def api_TestBox $50 + (3 * 6) ; HL position, DE size, A type (0-15, basically only filter by the high nibble of type), A returns dot ID (or 255 if no dot found)
.def api_RemoveDot $50 + (3 * 7) ;A dot id
.def api_SetDotType $50 + (3 * 8) ;A dot id, b new dot type
.def api_GetScreenCoord $50 + (3 * 9) ; HL global position, returns A as screen coord (or 255 as out of screen)